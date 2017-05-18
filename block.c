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
    block_r       block = (block_r) tree;
    tree_io_fn    io;
    void *        stream;
    tree_r        child;
    block_delim_p delim;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "block";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for blocks)
        return (tree_p) (sizeof(block_t));

    case TREE_ARITY:
        // Blocks have one child
        return (tree_p) 1;

    case TREE_CHILDREN:
        // Return pointer to only child
        return (tree_p) &block->child;

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see block_new)
        child = va_arg(va, tree_r);
        delim = va_arg(va, block_delim_p);

        // Create block and copy data in it
        block = (block_r) tree_malloc(sizeof(block_t));
        tree_set(&block->child, child);
        block->delimiters = delim;
        return (tree_p) block;

    case TREE_RENDER:
        // Render the opening and closing, with child inbetween
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        delim = block->delimiters;

        name_render(delim->opening, io, stream);
        tree_render(tree, io, stream);
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

block_delim_p block_paren  = NULL;
block_delim_p block_curly  = NULL;
block_delim_p block_square = NULL;
block_delim_p block_indent = NULL;
