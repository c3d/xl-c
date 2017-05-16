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


// C99 inline semantics is so broken it's not worth trying...
#define inline static __inline__ __attribute__((gnu_inline))



// ============================================================================
//
//    Forward declaration of the major tree types
//
// ============================================================================
//
//    XL code uses a const convention to help with garbage collection
//    - tree_p, aka const tree_t *, is used for referenced trees,
//      including all trees stored in a structure.
//    - tree_r (for raw), aka tree_t * is used for not-yet-referenced trees,
//      notably newly-created trees created by tree_new.
//
//    Before being placed in a structure, newly created trees must have
//    their reference count incremented, and they are then made const.
//    This minimizes the chances of just storing them by mistake in a
//    persistent structure without ref-counting them. Doing so is likely
//    to cause a compilation error or warning, since you discard constness.
//
//    Many constructors like prefix_new will accept tree_r inputs, since
//    they do the proper referencing inside. So they can be safely
//    used with either tree_p or tree_r.
//
//    A hint to remember this convention: 'const' means that you can't
//    modify it freely anymore, because someone else may reference it.
//

typedef const struct tree    *tree_p;
typedef const struct integer *integer_p;
typedef const struct real    *real_p;
typedef const struct blob    *blob_p;
typedef const struct text    *text_p;
typedef       struct text    *text_r;
typedef const struct name    *name_p;
typedef const struct block   *block_p;
typedef const struct pfix    *pfix_p;
typedef const struct prefix  *prefix_p;
typedef const struct postfix *postfix_p;
typedef const struct infix   *infix_p;
typedef const struct array   *array_p;



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
} tree_cmd_t;


// Handler function for tree structures
typedef tree_p (*tree_handler_fn)(tree_cmd_t cmd, tree_p tree, va_list va);

// Input and output functions, returns amount of data read or written
typedef unsigned (*tree_io_fn)(void *stream, unsigned sz, void *data);


typedef struct tree
// ----------------------------------------------------------------------------
//   Base tree structure
// ----------------------------------------------------------------------------
{
    tree_handler_fn     handler;      // Handler function for the tree
    unsigned            refcount;     // Reference count (garbage collection)
    unsigned            position;     // Source code position
} tree_t;

typedef       tree_t *tree_r;         // Non-persistent pointer
typedef const tree_t *tree_p;         // Persistent pointer (ref-counted)


// Allocate and initialize a new tree structure
extern tree_r      tree_make(tree_handler_fn handler, unsigned position, ...);
inline tree_r      tree_new(unsigned position);
inline void        tree_delete(tree_p tree);
inline unsigned    tree_ref(tree_p tree);
inline unsigned    tree_unref(tree_p tree);
inline tree_p      tree_use(tree_r tree);
inline void        tree_dispose(tree_p *tree);
inline const char *tree_typename(tree_p tree);
inline size_t      tree_size(tree_p tree);
inline size_t      tree_arity(tree_p tree);
inline tree_p *    tree_children(tree_p tree);
inline tree_p      tree_child(tree_p tree, unsigned index);
inline tree_p      tree_set_child(tree_p tree, unsigned index, tree_r child);
inline tree_r      tree_copy(tree_p tree);
inline tree_r      tree_clone(tree_p tree);
extern text_r      tree_text(tree_p tree);
extern bool        tree_print(FILE *stream, tree_p tree);
inline bool        tree_render(tree_p tree, tree_io_fn output, void *stream);
inline bool        tree_freeze(tree_p tree, tree_io_fn output, void *stream);
inline tree_r      tree_thaw(tree_io_fn input, void *stream);
extern tree_r      tree_io(tree_cmd_t cmd, tree_r tree, ...);

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

// Macro to create adapters for another type
#define tree_typedef(name)                                      \
                                                                \
typedef       name##_t *name##_r;                               \
typedef const name##_t *name##_p;                               \
                                                                \
inline void name##_delete(name##_p name)                        \
{                                                               \
    return tree_delete((tree_p) name);                          \
}                                                               \
                                                                \
inline unsigned name##_ref(name##_p name)                       \
{                                                               \
    return tree_ref((tree_p) name);                             \
}                                                               \
                                                                \
inline unsigned name##_unref(name##_p name)                     \
{                                                               \
    return tree_unref((tree_p) name);                           \
}                                                               \
                                                                \
inline name##_p name##_use(name##_r name)                       \
{                                                               \
    return (name##_p) tree_use((tree_r) name);                  \
}                                                               \
                                                                \
inline void name##_dispose(name##_p *name)                      \
{                                                               \
    tree_dispose((tree_p *) name);                              \
}                                                               \
                                                                \
inline const char *name##_typename(name##_p name)               \
{                                                               \
    return tree_typename((tree_p) name);                        \
}                                                               \
                                                                \
inline size_t name##_size(name##_p name)                        \
{                                                               \
    return tree_size((tree_p) name);                            \
}                                                               \
                                                                \
inline name##_r name##_copy(name##_p name)                      \
{                                                               \
    return (name##_r) tree_copy((tree_p) name);                 \
}                                                               \
                                                                \
inline name##_r name##_clone(name##_p name)                     \
{                                                               \
    return (name##_r) tree_clone((tree_p) name);                \
}                                                               \
                                                                \
inline text_r name##_text(name##_p name)                        \
{                                                               \
    return tree_text((tree_p) name);                            \
}                                                               \
                                                                \
                                                                \
inline bool name##_print(FILE *f, name##_p name)                \
{                                                               \
    return tree_print(f, (tree_p) name);                        \
}                                                               \
                                                                \
                                                                \
inline bool name##_render(name##_p name,                        \
                          tree_io_fn output, void *stream)      \
{                                                               \
    return tree_render((tree_p) name, output, stream);          \
}                                                               \
                                                                \
inline bool name##_freeze(name##_p name,                        \
                          tree_io_fn output, void *stream)      \
{                                                               \
    return tree_freeze((tree_p) name, output, stream);          \
}                                                               \
                                                                \
inline name##_r name##_thaw(tree_io_fn input, void *stream)     \
{                                                               \
    return (name##_r) tree_thaw(input, stream);                 \
}


// Macro to define a type that has children but overrides child / set_child
#define tree_children_typedef_override(name)                    \
                                                                \
tree_typedef(name);                                             \
                                                                \
inline size_t name##_arity(name##_p name)                       \
{                                                               \
    return tree_arity((tree_p) name);                           \
}                                                               \
                                                                \
inline tree_p *name##_children(name##_p name)                   \
{                                                               \
    return tree_children((tree_p) name);                        \
}                                                               \


#define tree_children_typedef(name)                             \
                                                                \
tree_children_typedef_override(name);                           \
                                                                \
inline tree_p name##_child(name##_p name, unsigned index)       \
{                                                               \
    return tree_child((tree_p) name, index);                    \
}                                                               \
                                                                \
inline tree_p name##_set_child(name##_p name,                   \
                               unsigned index, tree_r child)    \
{                                                               \
    return tree_set_child((tree_p) name, index, child);         \
}



// ============================================================================
//
//    Several of the functions above are best implemented inline
//
// ============================================================================

inline tree_r tree_new(unsigned position)
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
    tree_r t = (tree_r) tree;
#ifdef __GNUC__
    return __atomic_fetch_add(&t->refcount, 1, __ATOMIC_ACQUIRE);
#else // !__GNUC__
#warning "No atomic support for this compiler: no thread safety"
    return t->refcount++;
#endif // __GNUC__
}


inline unsigned tree_unref(tree_p tree)
// ----------------------------------------------------------------------------
//   Decrement reference count of the tree
// ----------------------------------------------------------------------------
{
    tree_r t = (tree_r) tree;
#ifdef __GNUC__
    unsigned count = __atomic_add_fetch(&t->refcount, -1, __ATOMIC_ACQUIRE);
#else // !__GNUC__
    unsigned count = --t->refcount;
#endif // __GNUC__
    return count;
}


inline tree_p tree_use(tree_r tree)
// ----------------------------------------------------------------------------
//   Return a reference to the tree with incremented refcount
// ----------------------------------------------------------------------------
{
    if (tree)
        tree_ref(tree);
    return (tree_p) tree;
}


inline void tree_dispose(tree_p *tree)
// ----------------------------------------------------------------------------
//   Check if tree can be freed, and if so, delete it
// ----------------------------------------------------------------------------
//   This returns NULL to help with common usage: foo = tree_dispose(foo);
{
    if (*tree)
    {
        if (tree_unref(*tree) == 0)
            tree_delete(*tree);
        *tree = NULL;
    }
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


inline tree_p tree_set_child(tree_p tree, unsigned index, tree_r child)
// ----------------------------------------------------------------------------
//   Update the given child in the tree
// ----------------------------------------------------------------------------
{
    assert(index < tree_arity(tree) && "Index must be valid for this tree");
    tree_p *children = tree_children(tree);
    if (child != children[index])
    {
        tree_ref(child);
        tree_dispose(&children[index]);
        children[index] = child;
    }
    return child;
}


inline tree_r tree_copy(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a shallow copy of the current tree
// ----------------------------------------------------------------------------
{
    return (tree_r) tree->handler(TREE_COPY, tree, NULL);
}


inline tree_r tree_clone(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a deep copy of the current tree
// ----------------------------------------------------------------------------
{
    return (tree_r) tree->handler(TREE_CLONE, tree, NULL);
}


inline bool tree_render(tree_p tree, tree_io_fn output, void *stream)
// ----------------------------------------------------------------------------
//   Invoke the render function in the handler, returns true if successful
// ----------------------------------------------------------------------------
{
    return tree_io(TREE_RENDER, (tree_r) tree, output, stream) == tree;
}


inline bool tree_freeze(tree_p tree, tree_io_fn output, void *stream)
// ----------------------------------------------------------------------------
//   Freeze (serialize) the tree and return true if successful
// ----------------------------------------------------------------------------
{
    return tree_io(TREE_FREEZE, (tree_r) tree, output, stream) == tree;
}


inline tree_r tree_thaw(tree_io_fn input, void *stream)
// ----------------------------------------------------------------------------
//   Thaw (deserialize) the tree from the given input
// ----------------------------------------------------------------------------
{
    return (tree_r) tree_io(TREE_THAW, NULL, input, stream);
}

#endif // TREE_H
