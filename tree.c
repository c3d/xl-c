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

#include "tree.h"
#include "text.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


tree_p tree_make(tree_handler_fn handler, unsigned position, ...)
// ----------------------------------------------------------------------------
//   Create a new tree with the given handler, position and pass extra args
// ----------------------------------------------------------------------------
{
    va_list va;

    // Pass the va to TREE_INITIALIZE for dynamic types, e.g. text
    va_start(va, position);
    tree_p tree = handler(TREE_INITIALIZE, NULL, va);
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
    tree_p result = tree->handler(cmd, tree, va);
    va_end(va);
    return result;
}


static unsigned tree_text_output(void *stream, unsigned size, const char *data)
// ----------------------------------------------------------------------------
//   Append incoming text to the text
// ----------------------------------------------------------------------------
{
    // Append to the text, and if it does not happen in place, update pointer
    text_p *output = (text_p *) stream;
    text_p input = *output;
    text_p copy = text_append(input, size, data);
    if (copy && copy != input)
        *output = copy;
    return size;
}


text_p tree_text(tree_p tree)
// ----------------------------------------------------------------------------
//   Convert the tree to text by using the render callback
// ----------------------------------------------------------------------------
{
    text_p result = text_cnew(tree->position, "");
    tree_render(tree, tree_text_output, &result);
    return result;
}


static unsigned tree_print_output(void *stream, unsigned size, const char *data)
// ----------------------------------------------------------------------------
//   Write data to the given FILE * stream
// ----------------------------------------------------------------------------
{
    FILE *output = (FILE *) stream;
    return fwrite(data, size, 1, output);
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

    case TREE_INITIALIZE:
        // Default initialization for trees
        return (tree_p) malloc(sizeof(tree_t));

    case TREE_DELETE:
        // Check if the tree has a non-zero arity. If so, unref children
        tree_children_loop(tree, tree_dispose(*child));

        // Free the memory associated with the tree
        free(tree);
        return NULL;

    case TREE_COPY:
    case TREE_CLONE:
        // Perform a shallow or deep copy of the tree
        size = tree_size(tree);
        copy = (tree_p) malloc(size);
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
        return tree;
    }
}

