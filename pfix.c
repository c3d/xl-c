// ****************************************************************************
//  pfix.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of prefix and postfix nodes
//     Prefix nodes represent +A or sin A
//     Postfix nodes represent A% or A km
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

#define PFIX_C
#include "pfix.h"
#include <stdlib.h>
#include <strings.h>


tree_p pfix_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The common handler for prefix and postfix
// ----------------------------------------------------------------------------
{
    pfix_r     pfix = (pfix_r) tree;
    tree_io_fn io;
    void *     stream;
    tree_r     left, right;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "pfix";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for pfixs)
        return (tree_p) (sizeof(pfix_t));

    case TREE_ARITY:
        // Prefix and postfix have two children
        return (tree_p) 2;

    case TREE_CHILDREN:
        // Pointer to the children is right after the tree 'header
        return tree + 1;

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see pfix_new)
        left = va_arg(va, tree_r);
        right = va_arg(va, tree_r);

        // Create pfix and copy data in it
        pfix = (pfix_r) tree_malloc(sizeof(pfix_t));
        tree_set(&pfix->left, left);
        tree_set(&pfix->right, right);
        return (tree_p) pfix;

    case TREE_RENDER:
        // Render left then right
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        tree_render(pfix->left, io, stream);
        tree_render(pfix->right, io, stream);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}


tree_p prefix_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The prefix handler
// ----------------------------------------------------------------------------
{
    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "prefix";

    default:
        // Other cases are handled correctly by the tree handler
        return pfix_handler(cmd, tree, va);
    }
}


tree_p postfix_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The prefix handler
// ----------------------------------------------------------------------------
{
    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "postfix";

    default:
        // Other cases are handled correctly by the tree handler
        return pfix_handler(cmd, tree, va);
    }
}
