// ****************************************************************************
//  array.c                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Implementation of arrays
//
//
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

#include "array.h"
#include <stdlib.h>
#include <strings.h>



array_p array_make(tree_handler_fn handler, unsigned position, va_list va)
// ----------------------------------------------------------------------------
//   Create a new tree with the given handler, position and pass extra args
// ----------------------------------------------------------------------------
{
    tree_p tree = handler(TREE_INITIALIZE, NULL, va);
    tree->handler = handler;
    tree->refcount = 0;
    tree->position = position;
    return (array_p) tree;
}


tree_p array_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for arrays deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    array_p       array = (array_p) tree;
    tree_io_fn    io;
    void *        stream;
    size_t        size;
    tree_p        child;
    tree_p *      children;
    array_delim_p delim;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "array";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for arrays)
        return (tree_p) (sizeof(array_t) + array->size * sizeof(tree_p));

    case TREE_ARITY:
        // Arrays have a variable number of children
        return (tree_p) array->size;

    case TREE_CHILDREN:
        // Return pointer to first child, which is right after the array
        return (tree_p) (array + 1);

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see array_new)
        delim = va_arg(va, array_delim_p);
        size = va_arg(va, size_t);

        // Create array and copy data in it
        array = (array_p) malloc(sizeof(array_t) + size * sizeof(tree_p));
        array->size = size;
        array->delimiters = delim;
        children = (tree_p *) (array + 1);
        while (size--)
        {
            child = va_arg(va, tree_p);
            *children++ = tree_refptr(child);
        }
        return (tree_p) array;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        delim = array->delimiters;
        size = array->size;
        children = (tree_p *) (array + 1);

        name_render(delim->opening, io, stream);
        while (size--)
        {
            tree_render(*children++, io, stream);
            if (size)
                name_render(delim->separating, io, stream);
        }
        name_render(delim->closing, io, stream);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}




// ============================================================================
//
//   Global variables
//
// ============================================================================

array_delim_p array_paren  = NULL;
array_delim_p array_curly  = NULL;
array_delim_p array_square = NULL;
array_delim_p array_indent = NULL;
