// ****************************************************************************
//  pfix.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//
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

#include "pfix.h"
#include <stdlib.h>
#include <strings.h>


tree_p pfix_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The common handler for prefix and postfix
// ----------------------------------------------------------------------------
{
    pfix_p     pfix = (pfix_p) tree;
    tree_io_fn io;
    void *     stream;
    tree_p     left, right;

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

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see pfix_new)
        left = va_arg(va, tree_p);
        right = va_arg(va, tree_p);

        // Create pfix and copy data in it
        pfix = (pfix_p) malloc(sizeof(pfix_t));
        pfix->left = tree_refptr(left);
        pfix->right = tree_refptr(right);
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
