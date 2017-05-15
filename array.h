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
    size_t        size;
} array_t;
tree_children_typedef_override(array);


inline array_r       array_new(unsigned position, ...);
inline tree_p        array_child(array_p array, size_t index);
inline tree_p        array_set_child(array_p array, size_t index, tree_r val);
inline array_delim_p array_delimiters(array_p array);

// Private array handler, should not be called directly in general
extern array_r array_make(tree_handler_fn h, unsigned pos, va_list va);
extern tree_p  array_handler(tree_cmd_t cmd, tree_p tree, va_list va);


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

inline array_r array_new(unsigned position, ...)
// ----------------------------------------------------------------------------
//    Allocate a array with the given data
// ----------------------------------------------------------------------------
{
    va_list va;
    va_start(va, position);
    array_r result = array_make(array_handler, position, va);
    va_end(va);
    return result;
}


inline tree_p array_child(array_p array, size_t index)
// ----------------------------------------------------------------------------
//   Return the data for the array
// ----------------------------------------------------------------------------
{
    assert(index < array->size && "Array index must be within bounds");
    tree_p *children = (tree_p *) (array + 1);
    return children[index];
}


inline tree_p array_set_child(array_p array, size_t index, tree_r child)
// ----------------------------------------------------------------------------
//   Return the data for the array
// ----------------------------------------------------------------------------
{
    assert(index < array->size && "Array index must be within bounds");
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

#endif // ARRAY_H
