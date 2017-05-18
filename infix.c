// ****************************************************************************
//  infix.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of infix nodes, like A+B or A and B
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

#include "infix.h"
#include "name.h"
#include <stdlib.h>
#include <strings.h>


tree_p infix_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The common handler for prefix and postfix
// ----------------------------------------------------------------------------
{
    infix_r    infix = (infix_r) tree;
    tree_io_fn io;
    void *     stream;
    tree_r     left, right;
    name_r     opcode;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "infix";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for infixs)
        return (tree_p) (sizeof(infix_t));

    case TREE_ARITY:
        // Prefix and postfix have three children
        return (tree_p) 3;

    case TREE_CHILDREN:
        // Pointer to the children is right after the tree 'header
        return tree + 1;

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see infix_new)
        opcode = va_arg(va, name_r);
        left = va_arg(va, tree_r);
        right = va_arg(va, tree_r);

        // Create infix and copy data in it
        infix = (infix_r) tree_malloc(sizeof(infix_t));
        tree_set(&infix->left, left);
        tree_set(&infix->right, right);
        name_set(&infix->opcode, opcode);
        return (tree_p) infix;

    case TREE_RENDER:
        // Render left then right
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);
        tree_render(infix->left, io, stream);
        name_render(infix->opcode, io, stream);
        tree_render(infix->right, io, stream);
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}
