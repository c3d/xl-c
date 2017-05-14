#ifndef TREE_H
#define TREE_H
// ****************************************************************************
//  tree.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Basic representation of the XL parse tree.
//
//     See the big comment at the top of parser.h for details about
//     the basics of XL tree representation
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

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>



// ============================================================================
//
//    Forward declaration of the major tree types
//
// ============================================================================

typedef struct tree    *tree_p;
typedef struct integer *integer_p;
typedef struct real    *real_p;
typedef struct blob    *blob_p;
typedef struct text    *text_p;
typedef struct name    *name_p;
typedef struct block   *block_p;
typedef struct pfix    *pfix_p;
typedef struct prefix  *prefix_p;
typedef struct postfix *postfix_p;
typedef struct infix   *infix_p;
typedef struct array   *array_p;



// ============================================================================
//
//    The base tree type
//
// ============================================================================

typedef enum tree_cmd
// ----------------------------------------------------------------------------
//   Commands that all info handlers must accept
// ----------------------------------------------------------------------------
{
    TREE_EVALUATE,                      // Evaluate the tree
    TREE_TYPENAME,                      // Return a unique type name for tree
    TREE_SIZE,                          // Return the size for the tree
    TREE_ARITY,                         // Return the arity for the tree
    TREE_CHILDREN,                      // Return pointer to children of tree
    TREE_INITIALIZE,                    // Initialized the tree (from tree_new)
    TREE_DELETE,                        // Delete the tree and its children
    TREE_COPY,                          // Shallow copy of the tree
    TREE_CLONE,                         // Deep copy of the tree
    TREE_RENDER,                        // Render tree in text form
    TREE_FREEZE,                        // Serialize tree
    TREE_THAW,                          // De-serialize tree
} tree_cmd_t, *tree_cmd_p;


// Handler function for tree structures
typedef tree_p (*tree_handler_fn)(tree_cmd_t cmd, tree_p tree, va_list va);

// Input and output functions, returns amount of data read or written
typedef unsigned (*tree_io_fn)(void *stream, unsigned sz, const char *data);


typedef struct tree
// ----------------------------------------------------------------------------
//   Base tree structure
// ----------------------------------------------------------------------------
{
    tree_handler_fn     handler;      // Handler function for the tree
    unsigned            refcount;     // Reference count (garbage collection)
    unsigned            position;     // Source code position
} tree_t;


// Allocate and initialize a new tree structure
extern tree_p      tree_make(tree_handler_fn handler, unsigned position, ...);
inline tree_p      tree_new(unsigned position);
inline void        tree_delete(tree_p tree);
inline unsigned    tree_ref(tree_p tree);
inline unsigned    tree_unref(tree_p tree);
inline tree_p      tree_refptr(tree_p tree);
inline tree_p      tree_dispose(tree_p tree);
inline const char *tree_typename(tree_p tree);
inline size_t      tree_size(tree_p tree);
inline size_t      tree_arity(tree_p tree);
inline tree_p *    tree_children(tree_p tree);
inline tree_p      tree_child(tree_p tree, unsigned index);
inline tree_p      tree_set_child(tree_p tree, unsigned index, tree_p child);
inline tree_p      tree_copy(tree_p tree);
inline tree_p      tree_clone(tree_p tree);
extern text_p      tree_text(tree_p tree);
extern bool        tree_print(FILE *stream, tree_p tree);
inline bool        tree_render(tree_p tree, tree_io_fn output, void *stream);
inline bool        tree_freeze(tree_p tree, tree_io_fn output, void *stream);
inline tree_p      tree_thaw(tree_io_fn input, void *stream);
extern tree_p      tree_io(tree_cmd_t cmd, tree_p tree, ...);

// Default handler for tree operations - Do not call directly
extern tree_p tree_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Macro to loop on tree children
#define tree_children_loop(tree, body)            \
    do                                            \
    {                                             \
        tree_p parent = (tree);                   \
        size_t arity = tree_arity(parent);        \
        tree_p *child = tree_children(tree);      \
        while (arity)                             \
        {                                         \
            body;                                 \
            child++;                              \
            arity--;                              \
        }                                         \
    } while(0)




// ============================================================================
//
//    Several of the functions above are best implemented inline
//
// ============================================================================

inline tree_p tree_new(unsigned position)
// ----------------------------------------------------------------------------
//   Create a new tree with the default tree handler
// ----------------------------------------------------------------------------
{
    return tree_make(tree_handler, position);
}


inline void tree_delete(tree_p tree)
// ----------------------------------------------------------------------------
//   Delete a tree by calling its handler
// ----------------------------------------------------------------------------
{
    tree->handler(TREE_DELETE, tree, NULL);
}


inline unsigned tree_ref(tree_p tree)
// ----------------------------------------------------------------------------
//   Increment reference count of the tree
// ----------------------------------------------------------------------------
{
#ifdef __GNUC__
    return __atomic_fetch_add(&tree->refcount, 1, __ATOMIC_ACQUIRE);
#else // !__GNUC__
#warning "No atomic support for this compiler: no thread safety"
    return tree->refcount++;
#endif // __GNUC__
}


inline unsigned tree_unref(tree_p tree)
// ----------------------------------------------------------------------------
//   Decrement reference count of the tree
// ----------------------------------------------------------------------------
{
#ifdef __GNUC__
    unsigned count = __atomic_add_fetch(&tree->refcount, -1, __ATOMIC_ACQUIRE);
#else // !__GNUC__
    unsigned count = --tree->refcount;
#endif // __GNUC__
    return count;
}


inline tree_p tree_refptr(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a reference to the tree with incremented refcount
// ----------------------------------------------------------------------------
{
    tree_ref(tree);
    return tree;
}


inline tree_p tree_dispose(tree_p tree)
// ----------------------------------------------------------------------------
//   Check if tree can be freed, and if so, delete it
// ----------------------------------------------------------------------------
{
    if (tree_unref(tree) == 0)
    {
        tree_delete(tree);
        tree = NULL;
    }
    return tree;
}


inline const char *tree_typename(tree_p tree)
// ----------------------------------------------------------------------------
//   Return the type name for the tree
// ----------------------------------------------------------------------------
{
    return (const char *) tree->handler(TREE_TYPENAME, tree, NULL);
}


inline size_t tree_size(tree_p tree)
// ----------------------------------------------------------------------------
//   Return the size of the tree in bytes
// ----------------------------------------------------------------------------
{
    return (size_t) tree->handler(TREE_SIZE, tree, NULL);
}


inline size_t tree_arity(tree_p tree)
// ----------------------------------------------------------------------------
//   Return the arity (number of children) of the tree in bytes
// ----------------------------------------------------------------------------
{
    return (size_t) tree->handler(TREE_ARITY, tree, NULL);
}


inline tree_p *tree_children(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a pointer to the children for that tree
// ----------------------------------------------------------------------------
{
    return (tree_p *) tree->handler(TREE_CHILDREN, tree, NULL);
}


inline tree_p tree_child(tree_p tree, unsigned index)
// ----------------------------------------------------------------------------
//    Return the child at the given index
// ----------------------------------------------------------------------------
{
    assert(index < tree_arity(tree) && "Index must be valid for this tree");
    tree_p *children = tree_children(tree);
    return children[index];
}


inline tree_p tree_set_child(tree_p tree, unsigned index, tree_p child)
// ----------------------------------------------------------------------------
//   Update the given child in the tree
// ----------------------------------------------------------------------------
{
    assert(index < tree_arity(tree) && "Index must be valid for this tree");
    tree_p *children = tree_children(tree);
    if (child != children[index])
    {
        tree_ref(child);
        tree_unref(children[index]);
        children[index] = child;
    }
    return child;
}


inline tree_p tree_copy(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a shallow copy of the current tree
// ----------------------------------------------------------------------------
{
    return tree->handler(TREE_COPY, tree, NULL);
}


inline tree_p tree_clone(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a deep copy of the current tree
// ----------------------------------------------------------------------------
{
    return tree->handler(TREE_CLONE, tree, NULL);
}



inline bool tree_render(tree_p tree, tree_io_fn output, void *stream)
// ----------------------------------------------------------------------------
//   Invoke the render function in the handler, returns true if successful
// ----------------------------------------------------------------------------
{
    return tree_io(TREE_RENDER, tree, output, stream) == tree;
}


inline bool tree_freeze(tree_p tree, tree_io_fn output, void *stream)
// ----------------------------------------------------------------------------
//   Freeze (serialize) the tree and return true if successful
// ----------------------------------------------------------------------------
{
    return tree_io(TREE_FREEZE, tree, output, stream) == tree;
}


inline tree_p tree_thaw(tree_io_fn input, void *stream)
// ----------------------------------------------------------------------------
//   Thaw (deserialize) the tree from the given input
// ----------------------------------------------------------------------------
{
    return tree_io(TREE_THAW, NULL, input, stream);
}

#endif // TREE_H
