#ifndef INFIX_H
#define INFIX_H
// ****************************************************************************
//  infix.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Infix nodes in the parse tree, e.g. A+B or A and B
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

#include "tree.h"
#include "text.h"

typedef struct infix
// ----------------------------------------------------------------------------
//    Internal representation of an infix
// ----------------------------------------------------------------------------
{
    tree_t        tree;
    tree_p        left, right;
    name_p        opcode;
} infix_t, *infix_p;
tree_children_typedef(infix);


inline infix_p      infix_new(unsigned position,
                              text_p opcode, tree_p left, tree_p right);
inline name_p       infix_opcode(infix_p infix);
inline tree_p       infix_left(infix_p infix);
inline tree_p       infix_right(infix_p infix);

// Private infix handler, should not be called directly in general
inline infix_p      infix_make(tree_handler_fn h, unsigned pos,
                               text_p opcode, tree_p left, tree_p right);
extern tree_p       infix_handler(tree_cmd_t cmd, tree_p tree, va_list va);



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline infix_p infix_make(tree_handler_fn handler, unsigned pos,
                          text_p opcode, tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//   Create a infix with the given parameters
// ----------------------------------------------------------------------------
{
    return (infix_p) tree_make(handler, pos, opcode, left, right);
}


inline infix_p infix_new(unsigned position,
                         text_p opcode, tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//    Allocate a prefix with the given children
// ----------------------------------------------------------------------------
{
    return infix_make(infix_handler, position, opcode, left, right);
}


inline name_p infix_opcode(infix_p infix)
// ----------------------------------------------------------------------------
//   Return the data for the infix
// ----------------------------------------------------------------------------
{
    return infix->opcode;
}


inline tree_p infix_left(infix_p infix)
// ----------------------------------------------------------------------------
//   Return the data for the infix
// ----------------------------------------------------------------------------
{
    return infix->left;
}


inline tree_p infix_right(infix_p infix)
// ----------------------------------------------------------------------------
//   Return the data for the infix
// ----------------------------------------------------------------------------
{
    return infix->right;
}

#endif // INFIX_H
