#ifndef ARRAY_H
#define ARRAY_H
// ****************************************************************************
//  array.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Arrays are inner nodes with variable arity, used to represent
//    possibly large sequences of objects, like [A, B, C, D, E].
//    Arrays do not have separators, but can inerit them from blocks
//
//
//
//
//
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "tree.h"
#include "name.h"


typedef struct array
// ----------------------------------------------------------------------------
//    Internal representation of a array
// ----------------------------------------------------------------------------
{
    tree_t tree;
    size_t length;
} array_t;

#ifdef ARRAY_C
#define inline extern inline
#endif

tree_arity_type(array);
inline array_p  array_new(srcpos_t position, size_t length, tree_p *data);
inline tree_p   array_child(array_p array, size_t index);
inline tree_p   array_set_child(array_p array, size_t index, tree_p val);
inline tree_p * array_data(array_p array);
inline size_t   array_length(array_p array);

extern void     array_append_data(array_p *, size_t count, tree_p *trees);
inline void     array_append(array_p *array, array_p other);
extern void     array_range(array_p *array, size_t start, size_t len);
inline void     array_push(array_p *array, tree_p value);
inline tree_p   array_top(array_p array);
inline void     array_pop(array_p *array);

// Sorting and searching
typedef int     (*compare_fn) (tree_p elem1, tree_p elem2);
extern void     array_sort(array_p array, compare_fn compare, size_t stride);
extern int      array_search(array_p array, tree_p key,
                             compare_fn compare, size_t stride);

// Private array handler, should not be called directly in general
extern tree_p  array_handler(tree_cmd_t cmd, tree_p tree, va_list va);
inline array_p array_make(tree_handler_fn h, srcpos_t, size_t, tree_p *data);

// Formatting of array rendering
extern name_p  array_opening, array_closing, array_separator;

#undef inline



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline array_p array_make(tree_handler_fn h,
                          srcpos_t pos, size_t sz, tree_p *data)
// ----------------------------------------------------------------------------
//   Create an array with the given parameters
// ----------------------------------------------------------------------------
{
    return (array_p) tree_make(h, pos, sz, data);
}


inline array_p array_new(srcpos_t position, size_t length, tree_p *data)
// ----------------------------------------------------------------------------
//    Allocate a array with the given data
// ----------------------------------------------------------------------------
{
    return array_make(array_handler, position, length, data);
}


inline tree_p array_child(array_p array, size_t index)
// ----------------------------------------------------------------------------
//   Return the data for the array
// ----------------------------------------------------------------------------
{
    assert(index < array->length && "Array index must be within bounds");
    tree_p *children = (tree_p *) (array + 1);
    return children[index];
}


inline tree_p array_set_child(array_p array, size_t index, tree_p child)
// ----------------------------------------------------------------------------
//   Return the data for the array
// ----------------------------------------------------------------------------
{
    assert(index < array->length && "Array index must be within bounds");
    tree_p *children = (tree_p *) (array + 1);
    if (child != children[index])
    {
        tree_ref(child);
        tree_dispose(&children[index]);
        children[index] = child;
    }
    return child;
}


inline tree_p *array_data(array_p array)
// ----------------------------------------------------------------------------
//   Return a pointer ot data in the array
// ----------------------------------------------------------------------------
{
    return (tree_p *) (array + 1);
}


inline size_t array_length(array_p array)
// ----------------------------------------------------------------------------
//   Return the length of the array
// ----------------------------------------------------------------------------
{
    return array->length;
}


inline void array_append(array_p *array, array_p array2)
// ----------------------------------------------------------------------------
//   Append one array to another
// ----------------------------------------------------------------------------
{
    return array_append_data(array,
                             array_length(array2),
                             (tree_p *) array_data(array2));
}


inline void array_push(array_p *array, tree_p value)
// ----------------------------------------------------------------------------
//    Push the given element at end of the array
// ----------------------------------------------------------------------------
{
    array_append_data(array, 1, (tree_p *) &value);
}


inline tree_p array_top(array_p array)
// ----------------------------------------------------------------------------
//   Return last element of array
// ----------------------------------------------------------------------------
{
    assert(array_length(array) && "Cannot return top of empty array");
    return array_child(array, array_length(array) - 1);
}

inline void array_pop(array_p *array)
// ----------------------------------------------------------------------------
//   Pop last element at end of array
// ----------------------------------------------------------------------------
{
    assert(array_length(*array) && "Can only pop from non-empty array");
    return array_range(array, 0, array_length(*array) - 1);
}



// ============================================================================
//
//   Definition of specific array types
//
// ============================================================================

#define array_type(item, type)                                          \
                                                                        \
    tree_type(type);                                                    \
                                                                        \
    inline type##_p type##_new(srcpos_t pos, size_t sz, item##_p *data) \
    {                                                                   \
        return (type##_p) array_new(pos, sz, (tree_p *) data);          \
    }                                                                   \
                                                                        \
    inline void type##_append(type##_p *t1, type##_p t2)                \
    {                                                                   \
        array_append((array_p *) t1, (array_p) t2);                     \
    }                                                                   \
                                                                        \
                                                                        \
    inline void type##_append_data(type##_p *type,                      \
                                   size_t sz, item##_p *data)           \
    {                                                                   \
        array_append_data((array_p *) type, sz, (tree_p *) data);       \
    }                                                                   \
                                                                        \
                                                                        \
    inline void type##_range(type##_p *t, size_t start, size_t len)     \
    {                                                                   \
        array_range((array_p *) t, start, len);                         \
    }                                                                   \
                                                                        \
    inline item##_p *type##_data(type##_p type)                         \
    {                                                                   \
        return (item##_p *) array_data((array_p) type);                 \
    }                                                                   \
                                                                        \
    inline size_t type##_length(type##_p type)                          \
    {                                                                   \
        return array_length((array_p) type);                            \
    }                                                                   \
                                                                        \
    inline void type##_push(type##_p * type, item##_p value)            \
    {                                                                   \
        array_push((array_p *) type, (tree_p) value);                   \
    }                                                                   \
                                                                        \
    inline item##_p type##_top(type##_p type)                           \
    {                                                                   \
        return (item##_p) array_top((array_p) type);                    \
    }                                                                   \
                                                                        \
    inline void type##_pop(type##_p *type)                              \
    {                                                                   \
        assert(type##_length(*type) && "Can only pop if non-empty");    \
        type##_range(type, 0, type##_length(*type)-1);                  \
    }

#endif // ARRAY_H
