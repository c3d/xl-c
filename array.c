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


void array_append_data(array_p *array_ptr, size_t sz, tree_p *data)
// ----------------------------------------------------------------------------
//   Append data to the array - In place if possible
// ----------------------------------------------------------------------------
//   This is very similar to blobs, but does ref-counting of elements
{
    array_p array = *array_ptr;
    array_p in_place = array;
    if (array_ref(array))
        in_place = NULL;
    size_t old_size = sizeof(array_t) + array->length * sizeof(tree_p);
    size_t ins_size = sz * sizeof(tree_p);
    size_t new_size = old_size + ins_size;

    array_p result = (array_p) tree_realloc((tree_p) in_place, new_size);
    if (result)
    {
        if (!in_place)
        {
            memcpy(result, array, old_size);
            result->tree.refcount = 0;

            // Since we make a new in_place, we must reference these items
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
    if (result != array)
        array_set(array_ptr, result);
}


void array_range(array_p *array_ptr, size_t first, size_t length)
// ----------------------------------------------------------------------------
//   Select a range of the array, in place if possible
// ----------------------------------------------------------------------------
//   We can move in place if there is only one user of this array
{
    array_p array = *array_ptr;
    array_p in_place = array;
    size_t end = first + length;
    if (end > array->length)
        end = array->length;
    if (first > array->length)
        first = array->length;
    size_t resized = end - first;
    size_t resized_bytes = sizeof(array_t) + resized * sizeof(tree_p);
    if (array_ref(array))
    {
        in_place = (array_p) tree_malloc(resized_bytes);
        memcpy(in_place, array, sizeof(array_t));
        in_place->tree.refcount = 0;
    }
    tree_p *src_data = array_data(array) + first;
    tree_p *dst_data = array_data(in_place);
    if (in_place == array)
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
        // In_Place data and incrementits reference count
        for (size_t i = 0; i < resized; i++)
            tree_set(&dst_data[i], (tree_p) src_data[i]);
    }
    in_place->length = resized;
    if (in_place == array)
    {
        // We did the in_place in place: need to truncate result
        in_place = (array_p) tree_realloc((tree_p) in_place, resized_bytes);
    }
    array_unref(array);
    if (in_place != array)
        array_set(array_ptr, in_place);
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
    tree_p *      children_src;
    tree_p *      children_dst;

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

    case TREE_CAST:
        // Check if we cast to array type, if so, success
        if (tree_cast_handler(va) == array_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see array_new)
        size = va_arg(va, size_t);
        children_src = va_arg(va, tree_p *);

        // Create array and copy data in it
        array = (array_p) tree_malloc(sizeof(array_t) + size * sizeof(tree_p));
        array->length = size;
        children_dst = (tree_p *) (array + 1);
        while (size--)
        {
            child = *children_src++;
            *children_dst++ = tree_use(child);
        }
        return (tree_p) array;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        size = array->length;
        children_src = (tree_p *) (array + 1);

        if (array_opening)
            name_render(array_opening, io, stream);
        while (size--)
        {
            tree_render(*children_src++, io, stream);
            if (size)
            {
                if (array_separator)
                    name_render(array_separator, io, stream);
                else
                    io(stream, 1, " ");
            }
        }
        if (array_closing)
            name_render(array_closing, io, stream);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        break;
    }
    return tree_handler(cmd, tree, va);
}




// ============================================================================
//
//   Global variables
//
// ============================================================================

name_p array_opening   = NULL;
name_p array_closing   = NULL;
name_p array_separator = NULL;
