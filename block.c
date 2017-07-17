// ****************************************************************************
//  block.c                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Implementation of blocks
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

#define BLOCK_C
#include "block.h"
#include "renderer.h"
#include <stdlib.h>
#include <strings.h>


void block_append_data(block_p *block_ptr, size_t sz, tree_p *data)
// ----------------------------------------------------------------------------
//   Append data to the block - In place if possible
// ----------------------------------------------------------------------------
//   This is very similar to blobs, but does ref-counting of elements
{
    block_p block = *block_ptr;
    block_p in_place = block;
    if (block_ref(block))
        in_place = NULL;
    size_t old_size = sizeof(block_t) + block->length * sizeof(tree_p);
    size_t ins_size = sz * sizeof(tree_p);
    size_t new_size = old_size + ins_size;

    block_p result = (block_p) tree_realloc((tree_p) in_place, new_size);
    if (result)
    {
        if (!in_place)
        {
            memcpy(result, block, old_size);
            result->tree.refcount = 0;

            // Since we make a new in_place, we must reference these items
            tree_p *children = block_data(result);
            size_t length = block_length(result);
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
    block_unref(block);
    if (result != block)
        block_set(block_ptr, result);
}


void block_range(block_p *block_ptr, size_t first, size_t length)
// ----------------------------------------------------------------------------
//   Select a range of the block, in place if possible
// ----------------------------------------------------------------------------
//   We can move in place if there is only one user of this block
{
    block_p block = *block_ptr;
    block_p in_place = block;
    size_t end = first + length;
    if (end > block->length)
        end = block->length;
    if (first > block->length)
        first = block->length;
    size_t resized = end - first;
    size_t resized_bytes = sizeof(block_t) + resized * sizeof(tree_p);
    if (block_ref(block))
    {
        in_place = (block_p) tree_malloc(resized_bytes);
        memcpy(in_place, block, sizeof(block_t));
        in_place->tree.refcount = 0;
    }
    tree_p *src_data = block_data(block) + first;
    tree_p *dst_data = block_data(in_place);
    if (in_place == block)
    {
        // Unreference all elements in block that we are not keeping
        for (size_t i = 0; i < first; i++)
            tree_dispose(&dst_data[i]);
        for (size_t i = end; i < block->length; i++)
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
    if (in_place == block)
    {
        // We did the in_place in place: need to truncate result
        in_place = (block_p) tree_realloc((tree_p) in_place, resized_bytes);
    }
    block_unref(block);
    if (in_place != block)
        block_set(block_ptr, in_place);
}


tree_p block_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blocks deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    block_p    block = (block_p) tree;
    renderer_p renderer;
    size_t     size;
    tree_p     child;
    tree_p *   children_src;
    tree_p *   children_dst;
    name_p     opening, closing, separator;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "block";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for blocks)
        return (tree_p) (sizeof(block_t) + block->length * sizeof(tree_p));

    case TREE_ARITY:
        // Blocks have a variable number of children
        return (tree_p) (block->length + 3);

    case TREE_CHILDREN:
        // Return pointer to first child, which is right after the block
        return (tree_p) &block->opening;

    case TREE_CAST:
        // Check if we cast to block type, if so, success
        if (tree_cast_handler(va) == block_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_INITIALIZE:
        // Fetch creation arguments from new
        opening = va_arg(va, name_p);
        closing = va_arg(va, name_p);
        separator = va_arg(va, name_p);
        size = va_arg(va, size_t);
        children_src = va_arg(va, tree_p *);

        // Create block and copy data in it
        block = (block_p) tree_malloc(sizeof(block_t) + size * sizeof(tree_p));
        block->length = size;
        block->opening = name_use(opening);
        block->closing = name_use(closing);
        block->separator = name_use(separator);
        children_dst = (tree_p *) (block + 1);
        while (size--)
        {
            child = *children_src++;
            *children_dst++ = tree_use(child);
        }
        return (tree_p) block;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        renderer = va_arg(va, renderer_p);
        size = block->length;
        children_src = (tree_p *) (block + 1);
        render(renderer, (tree_p) block->opening);
        while (size--)
        {
            render(renderer, *children_src++);
            if (size)
                render(renderer, (tree_p) block->separator);
        }
        render(renderer, (tree_p) block->closing);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        break;
    }
    return tree_handler(cmd, tree, va);
}
