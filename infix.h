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
} infix_t;


#ifdef INFIX_C
#define inline extern inline
#endif // INFIX_C

tree_children_type(infix);
inline infix_r      infix_new(srcpos_t position,
                              text_r opcode, tree_r left, tree_r right);
inline name_p       infix_opcode(infix_p infix);
inline tree_p       infix_left(infix_p infix);
inline tree_p       infix_right(infix_p infix);

// Private infix handler, should not be called directly in general
inline infix_r      infix_make(tree_handler_fn h, srcpos_t pos,
                               text_r opcode, tree_r left, tree_r right);
extern tree_p       infix_handler(tree_cmd_t cmd, tree_p tree, va_list va);

#undef inline



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline infix_r infix_make(tree_handler_fn handler, srcpos_t pos,
                          text_r opcode, tree_r left, tree_r right)
// ----------------------------------------------------------------------------
//   Create a infix with the given parameters
// ----------------------------------------------------------------------------
{
    return (infix_r ) tree_make(handler, pos, opcode, left, right);
}


inline infix_r infix_new(srcpos_t position,
                         text_r opcode, tree_r left, tree_r right)
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
