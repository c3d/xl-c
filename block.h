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


typedef struct block
// ----------------------------------------------------------------------------
//    Internal representation of a block
// ----------------------------------------------------------------------------
{
    tree_t        tree;
    tree_p        child;
    name_p        opening, closing;
} block_t;

#ifdef BLOCK_C
#define inline extern inline
#endif

tree_arity_type(block);
inline block_p block_new(srcpos_t position, tree_p chld, name_p opn, name_p cl);
inline tree_p  block_child(block_p block);
inline tree_p  block_set_child(block_p block, tree_p child);
inline name_p  block_opening(block_p block);
inline name_p  block_closing(block_p block);

// Private block handler, should not be called directly in general
inline block_p block_make(tree_handler_fn h, srcpos_t pos,
                          tree_p child, name_p open, name_p close);
extern tree_p  block_handler(tree_cmd_t cmd, tree_p tree, va_list va);

#undef inline



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline block_p block_make(tree_handler_fn handler, srcpos_t pos,
                          tree_p child, name_p open, name_p close)
// ----------------------------------------------------------------------------
//   Create a block with the given parameters
// ----------------------------------------------------------------------------
{
    return (block_p) tree_make(handler, pos, child, open, close);
}


inline block_p block_new(srcpos_t pos, tree_p child, name_p open, name_p close)
// ----------------------------------------------------------------------------
//    Allocate a block with the given data
// ----------------------------------------------------------------------------
{
    return block_make(block_handler, pos, child, open, close);
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
        block_p b = (block_p) block;
        tree_ref(child);
        tree_dispose(&b->child);
        b->child = child;
    }
    return child;
}


inline name_p block_opening(block_p block)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    return block->opening;
}


inline name_p block_closing(block_p block)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    return block->closing;
}

#endif // BLOCK_H
