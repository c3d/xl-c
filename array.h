#ifndef ARRAY_H
#define ARRAY_H
// ****************************************************************************
//  array.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Arrays are inner nodes with variable arity, used to represent large
//    sequeences of objects, like [A, B, C, D, E].
//    They are identified by opening, closing and separating symbols.
//
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


typedef struct array_delim
// ----------------------------------------------------------------------------
//   Array delimiters
// ----------------------------------------------------------------------------
{
    name_p      opening;
    name_p      separating;
    name_p      closing;
} array_delim_t, *array_delim_p;


typedef struct array
// ----------------------------------------------------------------------------
//    Internal representation of a array
// ----------------------------------------------------------------------------
{
    tree_t        tree;
    array_delim_p delimiters;
    size_t        length;
} array_t;

#ifdef ARRAY_C
#define inline extern inline
#endif

tree_arity_type(array);
inline array_r       array_new(srcpos_t position, array_delim_p delim,
                               size_t length, tree_r *data);
inline tree_p        array_child(array_p array, size_t index);
inline tree_p        array_set_child(array_p array, size_t index, tree_r val);
inline array_delim_p array_delimiters(array_p array);
inline tree_p *      array_data(array_p array);
inline size_t        array_length(array_p array);

extern array_p       array_append_data(array_p, size_t count, tree_r *trees);
inline array_p       array_append(array_p array, array_p other);
extern array_p       array_range(array_p array, size_t start, size_t len);
inline array_p       array_push(array_p array, tree_r value);
inline tree_p        array_top(array_p array);
inline array_p       array_pop(array_p array);


// Private array handler, should not be called directly in general
extern tree_p  array_handler(tree_cmd_t cmd, tree_p tree, va_list va);
inline array_r array_make(tree_handler_fn h, srcpos_t pos, array_delim_p delim,
                          size_t sz, tree_r *data);

#undef inline


// Standard array separators
extern array_delim_p array_paren, array_curly, array_square, array_indent;

#define paren_array_new(pos, size, ...)   array_new(pos, array_paren, size, ## __VA_ARGS__)
#define curly_array_new(pos, size, ...)   array_new(pos, array_curly, size, ## __VA_ARGS__)
#define square_array_new(pos, size, ...)  array_new(pos, array_square, size, ## __VA_ARGS__)
#define indent_array_new(pos, size, ...)  array_new(pos, array_indent, size, ## __VA_ARGS__)



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline array_r array_make(tree_handler_fn h, srcpos_t pos, array_delim_p delim,
                          size_t sz, tree_r *data)
// ----------------------------------------------------------------------------
//   Create an array with the given parameters
// ----------------------------------------------------------------------------
{
    return (array_r) tree_make(h, pos, delim, sz, data);
}


inline array_r array_new(srcpos_t position, array_delim_p delim,
                         size_t length, tree_r *data)
// ----------------------------------------------------------------------------
//    Allocate a array with the given data
// ----------------------------------------------------------------------------
{
    return array_make(array_handler, position, delim, length, data);
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


inline tree_p array_set_child(array_p array, size_t index, tree_r child)
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


inline array_delim_p array_delimiters(array_p array)
// ----------------------------------------------------------------------------
//   Return the data for the array
// ----------------------------------------------------------------------------
{
    return array->delimiters;
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


inline array_p array_append(array_p array, array_p array2)
// ----------------------------------------------------------------------------
//   Append one array to another
// ----------------------------------------------------------------------------
{
    return array_append_data(array,
                             array_length(array2),
                             (tree_r *) array_data(array2));
}


inline array_p array_push(array_p array, tree_r value)
// ----------------------------------------------------------------------------
//    Push the given element at end of the array
// ----------------------------------------------------------------------------
{
    return array_append_data(array, 1, (tree_r *) &value);
}


inline tree_p array_top(array_p array)
// ----------------------------------------------------------------------------
//   Return last element of array
// ----------------------------------------------------------------------------
{
    assert(array_length(array) && "Cannot return top of empty array");
    return array_child(array, array_length(array) - 1);
}

inline array_p array_pop(array_p array)
// ----------------------------------------------------------------------------
//   Pop last element at end of array
// ----------------------------------------------------------------------------
{
    assert(array_length(array) && "Can only pop from non-empty array");
    return array_range(array, 0, array_length(array) - 1);
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
    inline type##_r type##_new(srcpos_t pos, size_t sz, item##_r *data) \
    {                                                                   \
        return (type##_r) square_array_new(pos, sz, (tree_r *) data);   \
    }                                                                   \
                                                                        \
    inline type##_p type##_append(type##_p t1, type##_p t2)             \
    {                                                                   \
        return (type##_p) array_append((array_p)t1, (array_p)t2);       \
    }                                                                   \
                                                                        \
                                                                        \
    inline type##_p type##_append_data(type##_p type,                   \
                                       size_t sz, item##_r *data)       \
    {                                                                   \
        return (type##_p) array_append_data((array_p) type,             \
                                            sz, (tree_r *) data);       \
    }                                                                   \
                                                                        \
                                                                        \
    inline type##_p type##_range(type##_p t, size_t start, size_t len)  \
    {                                                                   \
        return (type##_p) array_range((array_p) t, start, len);         \
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
    inline type##_p type##_push(type##_p type, item##_r value)          \
    {                                                                   \
        return (type##_p) array_push((array_p) type, (tree_r) value);   \
    }                                                                   \
                                                                        \
    inline item##_p type##_top(type##_p type)                           \
    {                                                                   \
        return (item##_p) array_top((array_p) type);                    \
    }                                                                   \
                                                                        \
    inline type##_p type##_pop(type##_p type)                           \
    {                                                                   \
        assert(type##_length(type) && "Can only pop if non-empty");     \
        return type##_range(type, 0, type##_length(type)-1);            \
    }

#endif // ARRAY_H
