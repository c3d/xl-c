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

#define ARRAY_C
#include "array.h"
#include <stdlib.h>
#include <strings.h>


array_p array_append_data(array_p array, size_t sz, tree_r *data)
// ----------------------------------------------------------------------------
//   Append data to the array - In place if possible
// ----------------------------------------------------------------------------
//   This is very similar to blobs, but does ref-counting of elements
{
    array_r copy = (array_r) array;
    if (array_ref(array))
        copy = NULL;
    size_t old_size = sizeof(array_t) + array->length * sizeof(tree_p);
    size_t ins_size = sz * sizeof(tree_p);
    size_t new_size = old_size + ins_size;

    array_r result = (array_r) tree_realloc((tree_r) copy, new_size);
    if (result)
    {
        if (result != copy)
        {
            memcpy(result, array, old_size);
            result->tree.refcount = 0;

            // Since we make a new copy, we must reference these items
            tree_p *children = array_data(result);
            size_t length = array_length(result);
            for (size_t i = 0; i < length; i++)
                tree_ref(children[i]);
        }
        char *append_dst = (char *) result + old_size;
        if (data)
        {
            memcpy(append_dst, data, ins_size);
            for (size_t i = 0; i < sz; i++)
                tree_ref(data[i]);
        }
        else
        {
            // Initialize with 0
            memset(append_dst, 0, ins_size);
        }
        result->length += sz;
    }
    array_unref(array);
    return result;
}


array_p array_range(array_p array, size_t first, size_t length)
// ----------------------------------------------------------------------------
//   Select a range of the array, in place if possible
// ----------------------------------------------------------------------------
//   We can move in place if there is only one user of this array
{
    array_r copy = (array_r) array;
    size_t end = first + length;
    if (end > array->length)
        end = array->length;
    if (first > array->length)
        first = array->length;
    size_t resized = end - first;
    size_t resized_bytes = sizeof(array_t) + resized * sizeof(tree_p);
    if (array_ref(array))
    {
        copy = (array_r) tree_malloc(resized_bytes);
        memcpy(copy, array, sizeof(array_t));
        copy->tree.refcount = 0;
    }
    tree_p *src_data = array_data(array) + first;
    tree_p *dst_data = array_data(copy);
    if (copy == array)
    {
        // Unreference all elements in array that we are not keeping
        for (size_t i = 0; i < first; i++)
            tree_dispose(&dst_data[i]);
        for (size_t i = end; i < array->length; i++)
            tree_dispose(&dst_data[i]);

        // Remaining pointers can just be moved
        memmove(dst_data, src_data, resized * sizeof(tree_p));
    }
    else
    {
        // Copy data and incrementits reference count
        for (size_t i = 0; i < resized; i++)
            dst_data[i] = tree_use((tree_r) src_data[i]);
    }
    copy->length = resized;
    if (copy == array)
    {
        // We did the copy in place: need to truncate result
        copy = (array_r) tree_realloc((tree_r) copy, resized_bytes);
    }
    array_unref(array);
    return copy;
}


tree_p array_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for arrays deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    array_r       array = (array_r) tree;
    tree_io_fn    io;
    void *        stream;
    size_t        size;
    tree_r        child;
    tree_p *      children;
    array_delim_p delim;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "array";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for arrays)
        return (tree_p) (sizeof(array_t) + array->length * sizeof(tree_p));

    case TREE_ARITY:
        // Arrays have a variable number of children
        return (tree_p) array->length;

    case TREE_CHILDREN:
        // Return pointer to first child, which is right after the array
        return (tree_p) (array + 1);

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see array_new)
        delim = va_arg(va, array_delim_p);
        size = va_arg(va, size_t);
        children = va_arg(va, tree_p *);

        // Create array and copy data in it
        array = (array_r) tree_malloc(sizeof(array_t) + size * sizeof(tree_p));
        array->length = size;
        array->delimiters = delim;
        children = (tree_p *) (array + 1);
        while (size--)
        {
            child = va_arg(va, tree_r);
            *children++ = tree_use(child);
        }
        return (tree_p) array;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        delim = array->delimiters;
        size = array->length;
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
