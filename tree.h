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

// Macros to indicate source position
#define SOURCE__(L)         #L
#define SOURCE_(F,L)        (F ":" SOURCE__(L))
#define SOURCE              SOURCE_(__FILE__, __LINE__)



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
    TREE_CAST,                          // Check class of pointer
    TREE_INITIALIZE,                    // Initialized the tree (from tree_new)
    TREE_DELETE,                        // Delete the tree and its children
    TREE_COPY,                          // Shallow copy of the tree
    TREE_CLONE,                         // Deep copy of the tree
    TREE_RENDER,                        // Render tree in text form
    TREE_FREEZE,                        // Serialize tree
    TREE_THAW,                          // De-serialize tree
} tree_cmd_t;
extern const char *tree_cmd_name(tree_cmd_t);

// Handler function for tree structures
typedef tree_p (*tree_handler_fn)(tree_cmd_t cmd, tree_p tree, va_list va);

// Input and output functions, returns amount of data read or written
typedef unsigned (*tree_io_fn)(void *stream, unsigned sz, void *data);

// Position indicator in files
typedef uintptr_t srcpos_t;

// Reference counting
typedef uintptr_t refcnt_t;


typedef struct tree
// ----------------------------------------------------------------------------
//   Base tree structure
// ----------------------------------------------------------------------------
{
    tree_handler_fn     handler;      // Handler function for the tree
    refcnt_t            refcount;     // Reference count (garbage collection)
    srcpos_t            position;     // Source code position
} tree_t, *tree_p;

#ifdef TREE_C
#define inline extern inline
#endif // TREE_C

// Public interface for trees
inline tree_p      tree_new(srcpos_t position);
inline void        tree_delete(tree_p tree);
inline refcnt_t    tree_refcount(tree_p tree);
inline refcnt_t    tree_ref(tree_p tree);
inline refcnt_t    tree_unref(tree_p tree);
inline tree_p      tree_use(tree_p tree);
inline void        tree_set(tree_p *ptr, tree_p tree);
inline void        tree_dispose(tree_p *tree);
inline const char *tree_typename(tree_p tree);
inline size_t      tree_size(tree_p tree);
inline size_t      tree_arity(tree_p tree);
inline srcpos_t    tree_position(tree_p tree);
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
inline tree_p      tree_cast_(tree_p tree, ...);


// Internal tree operations - Normally no need to call directly
extern tree_p tree_handler(tree_cmd_t cmd, tree_p tree, va_list va);
extern tree_p tree_make(tree_handler_fn handler, srcpos_t position, ...);
extern void   tree_memcheck();
extern tree_p tree_malloc_(const char *where, size_t size);
extern tree_p tree_realloc_(const char *where, tree_p old, size_t new_size);
extern void   tree_free_(const char *where, tree_p tree);
inline tree_handler_fn tree_cast_handler(va_list va);
#define tree_malloc(sz)         tree_malloc_(SOURCE, (sz))
#define tree_realloc(old, sz)   tree_realloc_(SOURCE, (old), (sz))
#define tree_free(t)            tree_free_(SOURCE, (t))
#define tree_cast(type, tree)   tree_cast_(tree, type##_handler)

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


#ifdef TREE_C
#undef inline
#endif



// ============================================================================
//
//    Several of the functions above are best implemented inline
//
// ============================================================================

inline tree_p tree_new(srcpos_t position)
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


#ifdef __GNUC__

// GCC-compatible compiler: use built-in atomic operations
#define tree_fetch_add(Value, Offset)                        \
    __atomic_fetch_add(&Value, Offset, __ATOMIC_ACQUIRE)

#define tree_add_fetch(Value, Offset)                        \
    __atomic_add_fetch(&Value, Offset, __ATOMIC_ACQUIRE)

#define tree_compare_exchange(Value, Expected, New)                     \
    __atomic_compare_exchange_n(&Value, &Expected, New,                 \
                                0, __ATOMIC_RELEASE, __ATOMIC_RELAXED)

#else // ! __GNUC__

#warning "Compiler not supported yet - Not thread safe"
#define tree_fetch_add(Value, OFfset)   (Value += Offset)
#define tree_add_fetch(Value, Offset)   ((Value += Offset) + Offset)
#define tree_compare_exchange(Value, Expected, New)   Value = New

#endif


inline refcnt_t tree_refcount(tree_p tree)
// ----------------------------------------------------------------------------
//   Return reference count of the tree
// ----------------------------------------------------------------------------
{
    return tree ? tree->refcount : (refcnt_t) -1;
}


inline refcnt_t tree_ref(tree_p tree)
// ----------------------------------------------------------------------------
//   Increment reference count of the tree
// ----------------------------------------------------------------------------
{
    assert(tree->refcount + 1 != 0 && "Suspiciously too many references");
    return tree_fetch_add(tree->refcount, 1);
}


inline refcnt_t tree_unref(tree_p tree)
// ----------------------------------------------------------------------------
//   Decrement reference count of the tree
// ----------------------------------------------------------------------------
{
    assert(tree->refcount && "Cannot unref if never referenced");
    refcnt_t count = tree_add_fetch(tree->refcount, -1);
    return count;
}


inline void tree_dispose(tree_p *tree)
// ----------------------------------------------------------------------------
//   Check if tree can be freed, and if so, delete it
// ----------------------------------------------------------------------------
//   This returns NULL to help with common usage: foo = tree_dispose(foo);
{
    if (*tree)
    {
        if ((*tree)->refcount == 0 || tree_unref(*tree) == 0)
            tree_delete(*tree);
        *tree = NULL;
    }
}


inline tree_p tree_use(tree_p tree)
// ----------------------------------------------------------------------------
//   Return a tree after increasing ref-count (for initialization purpose)
// ----------------------------------------------------------------------------
{
    if (tree)
        tree_ref(tree);
    return tree;
}


inline void tree_set(tree_p *ptr, tree_p tree)
// ----------------------------------------------------------------------------
//   Return a reference to the tree with incremented refcount
// ----------------------------------------------------------------------------
{
    if (*ptr != tree)
    {
        if (tree)
            tree_ref(tree);
        if (*ptr)
            tree_dispose(ptr);
        *ptr = tree;
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


inline srcpos_t tree_position(tree_p tree)
// ----------------------------------------------------------------------------
//   Return the arity (number of children) of the tree in bytes
// ----------------------------------------------------------------------------
{
    return tree->position;
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
    tree_set(children + index, child);
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


inline tree_p tree_cast_(tree_p tree, ...)
// ----------------------------------------------------------------------------
//   Convert the tree to the type given by handler, or return NULL
// ----------------------------------------------------------------------------
{
    if (tree)
    {
        va_list va;
        va_start(va, tree);
        tree = tree->handler(TREE_CAST, tree, va);
        va_end(va);
    }
    return tree;
}


inline tree_handler_fn tree_cast_handler(va_list va)
// ----------------------------------------------------------------------------
//    Extract the handler function passed to tree_cast_
// ----------------------------------------------------------------------------
{
    va_list va2;
    va_copy(va2, va);
    tree_handler_fn result = va_arg(va2, tree_handler_fn);
    va_end(va2);
    return result;
}



// ============================================================================
//
//   Building derived types
//
// ============================================================================

// Macro to create adapters for another type
#define tree_type(type)                                                 \
                                                                        \
    typedef struct type *type##_p;                                      \
                                                                        \
    extern tree_p type##_handler(tree_cmd_t cmd,tree_p tree,va_list va);\
                                                                        \
                                                                        \
    inline void type##_delete(type##_p type)                            \
    {                                                                   \
        return tree_delete((tree_p) type);                              \
    }                                                                   \
                                                                        \
    inline refcnt_t type##_refcount(type##_p type)                      \
    {                                                                   \
        return tree_refcount((tree_p) type);                            \
    }                                                                   \
                                                                        \
    inline refcnt_t type##_ref(type##_p type)                           \
    {                                                                   \
        return tree_ref((tree_p) type);                                 \
    }                                                                   \
                                                                        \
    inline refcnt_t type##_unref(type##_p type)                         \
    {                                                                   \
        return tree_unref((tree_p) type);                               \
    }                                                                   \
                                                                        \
    inline type##_p type##_use(type##_p value)                          \
    {                                                                   \
        return (type##_p) tree_use((tree_p) value);                     \
    }                                                                   \
                                                                        \
    inline void type##_set(type##_p *type, type##_p value)              \
    {                                                                   \
        tree_set((tree_p *) type, (tree_p) value);                      \
    }                                                                   \
                                                                        \
    inline void type##_dispose(type##_p *type)                          \
    {                                                                   \
        tree_dispose((tree_p *) type);                                  \
    }                                                                   \
                                                                        \
    inline type##_p type##_cast(const void * tree)                      \
    {                                                                   \
        return (type##_p) tree_cast(type, (tree_p) tree);               \
    }                                                                   \
                                                                        \
    inline type##_p type##_ptr(const void * tree)                       \
    {                                                                   \
        type##_p ptr = (type##_p) tree;                                 \
        assert (ptr == type##_cast(tree)                                \
                && "Tree does not have expected type");                 \
        return ptr;                                                     \
    }                                                                   \
                                                                        \
    inline const char *type##_typename(type##_p type)                   \
    {                                                                   \
        return tree_typename((tree_p) type);                            \
    }                                                                   \
                                                                        \
    inline size_t type##_size(type##_p type)                            \
    {                                                                   \
        return tree_size((tree_p) type);                                \
    }                                                                   \
                                                                        \
    inline srcpos_t type##_position(type##_p type)                      \
    {                                                                   \
        return tree_position((tree_p) type);                            \
    }                                                                   \
                                                                        \
    inline type##_p type##_copy(type##_p type)                          \
    {                                                                   \
        return (type##_p) tree_copy((tree_p) type);                     \
    }                                                                   \
                                                                        \
    inline type##_p type##_clone(type##_p type)                         \
    {                                                                   \
        return (type##_p) tree_clone((tree_p) type);                    \
    }                                                                   \
                                                                        \
    inline text_p type##_text(type##_p type)                            \
    {                                                                   \
        return tree_text((tree_p) type);                                \
    }                                                                   \
                                                                        \
    inline bool type##_print(FILE *f, type##_p type)                    \
    {                                                                   \
        return tree_print(f, (tree_p) type);                            \
    }                                                                   \
                                                                        \
                                                                        \
    inline bool type##_render(type##_p type,                            \
                              tree_io_fn output, void *stream)          \
    {                                                                   \
        return tree_render((tree_p) type, output, stream);              \
    }                                                                   \
                                                                        \
    inline bool type##_freeze(type##_p type,                            \
                              tree_io_fn output, void *stream)          \
    {                                                                   \
        return tree_freeze((tree_p) type, output, stream);              \
    }                                                                   \
                                                                        \
    inline type##_p type##_thaw(tree_io_fn input, void *stream)         \
    {                                                                   \
        return (type##_p) tree_thaw(input, stream);                     \
    }


// Macro to define a type that has children but overrides child / set_child
#define tree_arity_type(type)                           \
                                                        \
    tree_type(type);                                    \
                                                        \
    inline size_t type##_arity(type##_p type)           \
    {                                                   \
        return tree_arity((tree_p) type);               \
    }                                                   \
                                                        \
    inline tree_p *type##_children(type##_p type)       \
    {                                                   \
        return tree_children((tree_p) type);            \
    }


// Macro to define a type that has children and uses regular child functions
#define tree_children_type(type)                                        \
                                                                        \
    tree_arity_type(type);                                              \
                                                                        \
    inline tree_p type##_child(type##_p type, unsigned index)           \
    {                                                                   \
        return tree_child((tree_p) type, index);                        \
    }                                                                   \
                                                                        \
    inline tree_p type##_set_child(type##_p type,                       \
                                   unsigned index, tree_p child)        \
    {                                                                   \
        return tree_set_child((tree_p) type, index, child);             \
    }


// ============================================================================
//
//    Debugging support
//
// ============================================================================

// Print a tree in the debugger
extern void debug(void *p);

#endif // TREE_H
