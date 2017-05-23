// ****************************************************************************
//  tree.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Implementation of the parse tree elements
//
//
//
//
//
//
//
// ****************************************************************************
//  (C) 1992-2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#define TREE_C
#include "tree.h"

#include "recorder.h"
#include "text.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef NDEBUG

typedef struct tree_debug
// ----------------------------------------------------------------------------
//   Header containing debug information for tree debugging
// ----------------------------------------------------------------------------
{
    const char *        source; // Allocation position in source
    struct tree_debug * previous; // Global chain of trees for memchecks
    struct tree_debug * next;
} tree_debug_t, *tree_debug_p;

// Global list of trees for memory allocation debugging
static tree_debug_p trees = NULL, trees_end = NULL;
static unsigned allocs = 0;

#endif


tree_p tree_malloc_(const char *source, size_t size)
// ----------------------------------------------------------------------------
//   Allocate a tree, clear refcount and insert in global list
// ----------------------------------------------------------------------------
{
#ifdef NDEBUG
    tree_p result = malloc(size);
#else
    tree_debug_p debug = malloc(sizeof(tree_debug_t) + size);
    tree_p result = (tree_p) (debug + 1);

    allocs++;
    debug->source = source;
    debug->next = NULL;
    debug->previous = trees_end;
    if (trees_end)
        trees_end->next = debug;
    else
        trees = debug;
    trees_end = debug;
#endif // NDEBUG

    RECORD(ALLOC, "%s: malloc(%zu)=%p", source, size, result);
    memset(result, 0, size);

    return result;
}


tree_p tree_realloc_(const char *source, tree_p old, size_t new_size)
// ----------------------------------------------------------------------------
//   Reallocate a tree
// ----------------------------------------------------------------------------
{
    if (!old)
        return tree_malloc(new_size);

    assert(old->refcount <= 1 && "Do not create dangling pointers to tree");

#ifdef NDEBUG
    tree_p result = realloc(old, new_size);
#else
    tree_debug_p old_dbg = (tree_debug_p) old - 1;
    tree_debug_p previous = old_dbg->previous;
    tree_debug_p next = old_dbg->next;
    tree_debug_p debug = realloc(old_dbg, sizeof(tree_debug_t) + new_size);
    tree_p result = (tree_p) (debug + 1);

    allocs++;
    if (debug != old_dbg)
    {
        if (next)
            next->previous = debug;
        else
            trees_end = debug;
        if (previous)
            previous->next = debug;
        else
            trees = debug;
    }
    debug->source = source;
#endif // NDEBUG

    RECORD(ALLOC, "%s: realloc(%p,%zu)=%p", source, old, new_size, result);

    return result;
}


void tree_free_(const char *source, tree_p tree)
// ----------------------------------------------------------------------------
//   Free a tree
// ----------------------------------------------------------------------------
{
    assert(tree->refcount == 0 && "Only non-referenced trees can be freed");
    RECORD(ALLOC, "%s: free(%p) refcount %u", source, tree, tree->refcount);
#ifndef NDEBUG
    tree_debug_p debug = (tree_debug_p) tree - 1;
    tree_debug_p previous = debug->previous;
    tree_debug_p next = debug->next;
    if (previous)
        previous->next = next;
    else
        trees = next;
    if (next)
        next->previous = previous;
    else
        trees_end = previous;
    tree->handler = (tree_handler_fn) abort;
    tree->position = (srcpos_t) source;
    free(debug);
#else
    free(tree);
#endif // NDEBUG

}


void tree_memcheck()
// ----------------------------------------------------------------------------
//   Check the list of trees to see if we have non-referenced trees in it
// ----------------------------------------------------------------------------
//   This can be called at any point doing memory allocations, after
//   all trees have been tree_use'd or disposed of.
//   It will signal any leftover (leaked) tree.
{
#ifndef NDEBUG
    unsigned index = 0;
    bool bad = false;
    for (tree_debug_p debug = trees; debug; debug = debug->next)
    {
        index++;
        tree_p tree = (tree_p) (debug + 1);
        if ((int) tree->refcount <= 0)
        {
            fprintf(stderr,
                    "%s: Tree #%u (%p) has refcount %d\n",
                    debug->source, index, tree, (int) tree->refcount);
            bad = true;
        }
        if (index > allocs)
        {
            fprintf(stderr,
                    "*** More trees (%u) than what we allocated (%u)\n"
                    "*** Maybe a corruption of the list of trees\n",
                    index, allocs);
            bad = true;
            break;
        }
    }

    if (bad)
        recorder_dump();
#endif // NDEBUG
}


tree_p tree_make(tree_handler_fn handler, srcpos_t position, ...)
// ----------------------------------------------------------------------------
//   Create a new tree with the given handler, position and pass extra args
// ----------------------------------------------------------------------------
{
    va_list va;

    // Pass the va to TREE_INITIALIZE for dynamic types, e.g. text
    va_start(va, position);
    tree_p tree = (tree_p) handler(TREE_INITIALIZE, NULL, va);
    va_end(va);

    tree->handler = handler;
    tree->refcount = 0;
    tree->position = position;

    return tree;
}


tree_p tree_io(tree_cmd_t cmd, tree_p tree, ...)
// ----------------------------------------------------------------------------
//   Perform some tree I/O operation, passed over using varargs
// ----------------------------------------------------------------------------
{
    va_list va;
    va_start(va, tree);                    // Should really be (io, stream)
    tree_p result = (tree_p) tree->handler(cmd, tree, va);
    va_end(va);
    return result;
}


static unsigned tree_text_output(void *stream, unsigned size, void *data)
// ----------------------------------------------------------------------------
//   Append incoming text to the text
// ----------------------------------------------------------------------------
{
    // Append to the text, and if it does not happen in place, update pointer
    text_p *output = (text_p *) stream;
    text_append_data(output, size, (char *) data);
    return size;
}


text_p tree_text(tree_p tree)
// ----------------------------------------------------------------------------
//   Convert the tree to text by using the render callback
// ----------------------------------------------------------------------------
{
    if (!tree)
        return text_cnew(0, "<null>");
    text_p result = text_cnew(tree->position, "");
    tree_render(tree, tree_text_output, &result);
    return result;
}


static unsigned tree_print_output(void *stream, unsigned size, void *data)
// ----------------------------------------------------------------------------
//   Write data to the given FILE * stream
// ----------------------------------------------------------------------------
{
    FILE *output = (FILE *) stream;
    return fwrite(data, 1, size, output);
}


bool tree_print(FILE *stream, tree_p tree)
// ----------------------------------------------------------------------------
//    Print the tree to the given file output (typically stdout)
// ----------------------------------------------------------------------------
{
    return tree_render(tree, tree_print_output, stream);
}


tree_p tree_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The default type handler for base trees
// ----------------------------------------------------------------------------
{
    tree_p          copy;
    size_t          size;
    tree_io_fn      io;
    void *          stream;
    char            buffer[32];

    switch(cmd)
    {
    case TREE_EVALUATE:
        // Default evaluation for trees is to return the tree itself
        return tree;

    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "tree";

    case TREE_SIZE:
        // Return the size of the tree in bytes (may be dynamic for subtypes)
        return (tree_p) sizeof(tree_t);

    case TREE_ARITY:
        // Return the arity of the tree (number of children, 0 for leaves)
        return (tree_p) 0;

    case TREE_CHILDREN:
        // Return the pointer to children for that tree type
        return NULL;            // None if arity 0

    case TREE_CAST:
        // Return the tree if we cast to tree
        if (tree_cast_handler(va) == tree_handler)
            return tree;
        return NULL;            // Casting to some other type we did not reach

    case TREE_INITIALIZE:
        // Default initialization for trees
        return (tree_p) tree_malloc(sizeof(tree_t));

    case TREE_DELETE:
        // Check if the tree has a non-zero arity. If so, unref children
        tree_children_loop(tree, tree_dispose(child));

        // Free the memory associated with the tree
        assert(tree->refcount == 0 && "Cannot free tree if still referenced");
        tree_free((tree_p)tree);
        return NULL;

    case TREE_COPY:
    case TREE_CLONE:
        // Perform a shallow or deep copy of the tree
        size = tree_size(tree);
        copy = (tree_p) tree_malloc(size);
        if (copy)
        {
            memcpy(copy, tree, size);
            copy->refcount = 1;
            if (cmd == TREE_COPY)
                tree_children_loop(copy, tree_ref(*child));
            else
                tree_children_loop(copy, *child = tree_clone(*child));
        }
        return copy;

    case TREE_RENDER:
        // Default rendering simply shows the tree address and handler address
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        size = snprintf(buffer, sizeof(buffer),
                        "<%s:%p:%p>",
                        tree_typename(tree), tree,tree->handler);
        if (io(stream, size, buffer) != size)
            return NULL;        // Some error happened, report
        return tree;

    case TREE_FREEZE:
        assert(!"Not yet implemented");
        return tree;
    case TREE_THAW:
        assert(!"Not yet implemented");
        return tree;

    default:
        assert("Command not implemented");
        break;
    }
    return tree;
}


void debug(void *p)
// ----------------------------------------------------------------------------
//   For use in the debugger
// ----------------------------------------------------------------------------
{
    tree_print(stderr, (tree_p) p);
}
