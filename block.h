#ifndef BLOCK_H
#define BLOCK_H
// ****************************************************************************
//  block.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Blocks are inner nodes with arity 1, used to represent
//    things like (A), [A,B,C], {A;B;C} or indented blocks. They are identified
//    by opening, closing and separating symbols.
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


typedef struct block_delim
// ----------------------------------------------------------------------------
//   Block delimiters
// ----------------------------------------------------------------------------
{
    name_p      opening;
    name_p      closing;
} block_delim_t, *block_delim_p;


typedef struct block
// ----------------------------------------------------------------------------
//    Internal representation of a block
// ----------------------------------------------------------------------------
{
    tree_t        tree;
    block_delim_p delimiters;
    tree_p        child;
} block_t, *block_p;
tree_children_typedef_override(block);


inline block_p       block_new(unsigned position, tree_p child, block_delim_p);
inline tree_p        block_child(block_p block);
inline tree_p        block_set_child(block_p block, tree_p child);
inline block_delim_p block_delimiters(block_p block);

// Private block handler, should not be called directly in general
inline block_p block_make(tree_handler_fn h, unsigned pos,
                          tree_p child, block_delim_p);
extern tree_p  block_handler(tree_cmd_t cmd, tree_p tree, va_list va);


// Standard block separators
extern block_delim_p block_paren, block_curly, block_square, block_indent;

#define paren_new(pos, child)   block_new(pos, child, block_paren)
#define curly_new(pos, child)   block_new(pos, child, block_curly)
#define square_new(pos, child)  block_new(pos, child, block_square)
#define indent_new(pos, child)  block_new(pos, child, block_indent)



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline block_p block_make(tree_handler_fn handler, unsigned pos,
                          tree_p child, block_delim_p delim)
// ----------------------------------------------------------------------------
//   Create a block with the given parameters
// ----------------------------------------------------------------------------
{
    return (block_p) tree_make(handler, pos, child, delim);
}


inline block_p block_new(unsigned position, tree_p child, block_delim_p delim)
// ----------------------------------------------------------------------------
//    Allocate a block with the given data
// ----------------------------------------------------------------------------
{
    return block_make(block_handler, position, child, delim);
}


inline tree_p block_child(block_p block)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    return block->child;
}


inline tree_p block_set_child(block_p block, tree_p child)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    if (child != block->child)
    {
        tree_ref(child);
        tree_dispose(&block->child);
        block->child = child;
    }
    return child;
}


inline block_delim_p block_delimiters(block_p block)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    return block->delimiters;
}

#endif // BLOCK_H
