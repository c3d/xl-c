#ifndef PFIX_H
#define PFIX_H
// ****************************************************************************
//  pfix.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Implementation of prefix and postfix nodes
//    A prefix is a node like +A or sin X, with a named operator on the left
//    A postfix is a node like A% or 3km, with a named operator on the right
//    A pfix is a node like (X->1)(X) where neither left nor right is a name
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
tree_children_typedef(pfix);

inline pfix_p       pfix_new(unsigned position, tree_p left, tree_p right);
inline void         pfix_delete(pfix_p pfix);
inline tree_p       pfix_left(pfix_p pfix);
inline tree_p       pfix_right(pfix_p pfix);


typedef struct prefix *prefix_p;
tree_children_typedef(prefix);

inline prefix_p     prefix_new(unsigned position, name_p left, tree_p right);
inline name_p       prefix_operator(prefix_p prefix);
inline tree_p       prefix_operand(prefix_p prefix);


typedef struct postfix *postfix_p;
tree_children_typedef(postfix);

inline postfix_p    postfix_new(unsigned position, tree_p left, name_p right);
inline name_p       postfix_operator(postfix_p postfix);
inline tree_p       postfix_operand(postfix_p postfix);


// Private pfix handler, should not be called directly in general
inline pfix_p       pfix_make(tree_handler_fn h, unsigned pos,
                              tree_p left, tree_p right);
extern tree_p       pfix_handler(tree_cmd_t cmd, tree_p tree, va_list va);
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


inline pfix_p pfix_new(unsigned position, tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//    Allocate a prefix with the given children
// ----------------------------------------------------------------------------
//    This is used when neither left nor right is a name
//    In that case, the left applies to the right
{
    return pfix_make(prefix_handler, position, left, right);
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


inline prefix_p prefix_new(unsigned position, name_p left, tree_p right)
// ----------------------------------------------------------------------------
//    Allocate a prefix with the given children
// ----------------------------------------------------------------------------
{
    return (prefix_p) pfix_make(prefix_handler, position, (tree_p) left, right);
}


inline name_p prefix_operator(prefix_p pfix)
// ----------------------------------------------------------------------------
//   Return the operator for the prefix, a name in the left node
// ----------------------------------------------------------------------------
{
    return (name_p) ((pfix_p) pfix)->left;
}


inline tree_p prefix_operand(prefix_p pfix)
// ----------------------------------------------------------------------------
//   Return the operand(s) for the prefix, the tree on the right
// ----------------------------------------------------------------------------
{
    return ((pfix_p) pfix)->right;
}


inline postfix_p postfix_new(unsigned position, tree_p left, name_p right)
// ----------------------------------------------------------------------------
//    Allocate a postfix with the given children
// ----------------------------------------------------------------------------
{
    return (postfix_p) pfix_make(postfix_handler, position, left,(tree_p)right);
}


inline name_p postfix_operator(postfix_p pfix)
// ----------------------------------------------------------------------------
//   Return the operator for the postfix, a name in the right node
// ----------------------------------------------------------------------------
{
    return (name_p) ((pfix_p) pfix)->right;
}


inline tree_p postfix_operand(postfix_p pfix)
// ----------------------------------------------------------------------------
//   Return the operand(s) for the postfix, the tree on the left
// ----------------------------------------------------------------------------
{
    return ((pfix_p) pfix)->left;
}



#endif // PFIX_H
