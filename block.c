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

#include "block.h"
#include <stdlib.h>
#include <strings.h>


tree_p block_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blocks deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    block_p       block = (block_p) tree;
    size_t        size;
    tree_io_fn    io;
    void *        stream;
    tree_p        child;
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

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see block_new)
        child = va_arg(va, tree_p);
        delim = va_arg(va, block_delim_p);

        // Create block and copy data in it
        block = (block_p) malloc(sizeof(block_t));
        block->child = child;
        block->delimiters = delim;
        return (tree_p) block;

    case TREE_RENDER:
        // Dump the block as an hexadecimal string
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        delim = block->delimiters;

        size = strlen(delim->opening);
        if (io(stream, size, delim->opening) != size)
            return NULL;
        tree_render(tree, io, stream);
        size = strlen(delim->closing);
        if (io(stream, size, delim->closing) != size)
            return NULL;
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

static block_delim_t block_paren_data =  { "(", ")" };
static block_delim_t block_curly_data =  { "{", "}" };
static block_delim_t block_square_data = { "[", "]" };
static block_delim_t block_indent_data = { "\t", "\t" };

block_delim_p block_paren  = &block_paren_data;
block_delim_p block_curly  = &block_curly_data;
block_delim_p block_square = &block_square_data;
block_delim_p block_indent = &block_indent_data;
