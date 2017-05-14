#ifndef PFIX_H
#define PFIX_H
// ****************************************************************************
//  pfix.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Implementation of prefix and postfix nodes
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


typedef struct pfix
// ----------------------------------------------------------------------------
//    Internal representation of a pfix
// ----------------------------------------------------------------------------
{
    tree_t        tree;
    tree_p        left, right;
} pfix_t, *pfix_p;


inline pfix_p       prefix_new(unsigned position, tree_p left, tree_p right);
inline pfix_p       postfix_new(unsigned position, tree_p left, tree_p right);
inline void         pfix_delete(pfix_p pfix);
inline tree_p       pfix_left(pfix_p pfix);
inline tree_p       pfix_right(pfix_p pfix);

// Private pfix handler, should not be called directly in general
inline pfix_p       pfix_make(tree_handler_fn h, unsigned pos,
                              tree_p left, tree_p right);
extern tree_p       prefix_handler(tree_cmd_t cmd, tree_p tree, va_list va);
extern tree_p       postfix_handler(tree_cmd_t cmd, tree_p tree, va_list va);



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline pfix_p pfix_make(tree_handler_fn handler, unsigned pos,
                          tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//   Create a pfix with the given parameters
// ----------------------------------------------------------------------------
{
    return (pfix_p) tree_make(handler, pos, left, right);
}


inline pfix_p prefix_new(unsigned position, tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//    Allocate a prefix with the given children
// ----------------------------------------------------------------------------
{
    return pfix_make(prefix_handler, position, left, right);
}


inline pfix_p postfix_new(unsigned position, tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//    Allocate a postfix with the given children
// ----------------------------------------------------------------------------
{
    return pfix_make(postfix_handler, position, left, right);
}


inline void pfix_delete(pfix_p pfix)
// ----------------------------------------------------------------------------
//   Delete the given pfix
// ----------------------------------------------------------------------------
{
    tree_delete((tree_p) pfix);
}


inline tree_p pfix_left(pfix_p pfix)
// ----------------------------------------------------------------------------
//   Return the data for the pfix
// ----------------------------------------------------------------------------
{
    return pfix->left;
}


inline tree_p pfix_right(pfix_p pfix)
// ----------------------------------------------------------------------------
//   Return the data for the pfix
// ----------------------------------------------------------------------------
{
    return pfix->right;
}

#endif // PFIX_H
