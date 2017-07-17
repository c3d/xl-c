#ifndef BLOCK_H
#define BLOCK_H
// ****************************************************************************
//  block.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Blocks are like arrays, but with additional syntatic delimiters.
//    They are used to represent things like (A), [A,B,C], {A;B;C} or
//    indented blocks.
//    A block is delimited by
//    - an opening symbol, e.g.  [ { (
//    - a separator symbol, e.g. , ; \n
//    - a closing symbol, e.g.   ] } )
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
    tree_t tree;
    size_t length;
    name_p opening, closing, separator;
} block_t;

#ifdef BLOCK_C
#define inline extern inline
#endif

tree_arity_type(block);
inline block_p  block_new(srcpos_t position, name_p opening, name_p closing);
inline tree_p   block_child(block_p block, size_t index);
inline tree_p   block_set_child(block_p block, size_t index, tree_p val);
inline tree_p * block_data(block_p block);
inline size_t   block_length(block_p block);

extern void     block_append_data(block_p *, size_t count, tree_p *trees);
inline void     block_append(block_p *block, block_p other);
extern void     block_range(block_p *block, size_t start, size_t len);
inline void     block_push(block_p *block, tree_p value);
inline tree_p   block_top(block_p block);
inline void     block_pop(block_p *block);

inline name_p   block_opening(block_p block);
inline name_p   block_closing(block_p block);
inline name_p   block_separator(block_p block);

// Private block handler, should not be called directly in general
extern tree_p  block_handler(tree_cmd_t cmd, tree_p tree, va_list va);
inline block_p block_make(tree_handler_fn h, srcpos_t,
                          name_p opening, name_p closing, name_p separator,
                          size_t, tree_p *data);

#undef inline



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline block_p block_make(tree_handler_fn h, srcpos_t pos,
                          name_p opening, name_p closing, name_p separator,
                          size_t sz, tree_p *data)
// ----------------------------------------------------------------------------
//   Create an block with the given parameters
// ----------------------------------------------------------------------------
{
    return (block_p) tree_make(h, pos, opening, closing, separator, sz, data);
}


inline block_p block_new(srcpos_t position, name_p open, name_p close)
// ----------------------------------------------------------------------------
//    Allocate a block with the given data
// ----------------------------------------------------------------------------
{
    return block_make(block_handler, position, open, close, NULL, 0, NULL);
}


inline tree_p block_child(block_p block, size_t index)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
//   WARNING: index does not match that of tree_child.
//   block_child(block, N) == tree_child(block, N+3)
//   That's because tree_child also includes opening, closing and separator
{
    assert(index < block->length && "Block index must be within bounds");
    tree_p *children = (tree_p *) (block + 1);
    return children[index];
}


inline tree_p block_set_child(block_p block, size_t index, tree_p child)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
//   WARNING: index does not match that of tree_child.
//   block_child(block, N) == tree_child(block, N+3)
//   That's because tree_child also includes opening, closing and separator
{
    assert(index < block->length && "Block index must be within bounds");
    tree_p *children = (tree_p *) (block + 1);
    if (child != children[index])
    {
        tree_ref(child);
        tree_dispose(&children[index]);
        children[index] = child;
    }
    return child;
}


inline tree_p *block_data(block_p block)
// ----------------------------------------------------------------------------
//   Return a pointer ot data in the block
// ----------------------------------------------------------------------------
{
    return (tree_p *) (block + 1);
}


inline size_t block_length(block_p block)
// ----------------------------------------------------------------------------
//   Return the length of the block
// ----------------------------------------------------------------------------
{
    return block->length;
}


inline void block_append(block_p *block, block_p block2)
// ----------------------------------------------------------------------------
//   Append one block to another
// ----------------------------------------------------------------------------
{
    return block_append_data(block,
                             block_length(block2),
                             (tree_p *) block_data(block2));
}


inline void block_push(block_p *block, tree_p value)
// ----------------------------------------------------------------------------
//    Push the given element at end of the block
// ----------------------------------------------------------------------------
{
    block_append_data(block, 1, (tree_p *) &value);
}


inline tree_p block_top(block_p block)
// ----------------------------------------------------------------------------
//   Return last element of block
// ----------------------------------------------------------------------------
{
    assert(block_length(block) && "Cannot return top of empty block");
    return block_child(block, block_length(block) - 1);
}

inline void block_pop(block_p *block)
// ----------------------------------------------------------------------------
//   Pop last element at end of block
// ----------------------------------------------------------------------------
{
    assert(block_length(*block) && "Can only pop from non-empty block");
    return block_range(block, 0, block_length(*block) - 1);
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

inline name_p block_separator(block_p block)
// ----------------------------------------------------------------------------
//   Return the data for the block
// ----------------------------------------------------------------------------
{
    return block->separator;
}

#endif // BLOCK_H
