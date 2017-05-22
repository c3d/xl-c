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
#include <stdlib.h>
#include <strings.h>


tree_p block_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blocks deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    block_p    block = (block_p) tree;
    tree_io_fn io;
    void *     stream;
    tree_p     child;
    name_p     open, close;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "block";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for blocks)
        return (tree_p) (sizeof(block_t));

    case TREE_ARITY:
        // Blocks have three children (including opening and closing names)
        return (tree_p) 3;

    case TREE_CAST:
        // Check if we cast to block type, if so, success
        if (tree_cast_handler(va) == block_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_CHILDREN:
        // Return pointer to children
        return (tree_p) (tree + 1);

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see block_new)
        child = va_arg(va, tree_p);
        open = va_arg(va, name_p);
        close = va_arg(va, name_p);

        // Create block and copy data in it
        block = (block_p) tree_malloc(sizeof(block_t));
        block->child = tree_use(child);
        block->opening = name_use(open);
        block->closing = name_use(close);
        return (tree_p) block;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

        name_render(block->opening, io, stream);
        tree_render(block->child, io, stream);
        name_render(block->closing, io, stream);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        break;
    }
    return tree_handler(cmd, tree, va);
}
