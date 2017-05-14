// ****************************************************************************
//  text.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of text in the XL compiler
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

#include "text.h"
#include <stdlib.h>
#include <string.h>


tree_p text_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for texts deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    text_p        text = (text_p) tree;
    size_t        size;
    tree_io_fn    io;
    const char  * data;
    void *        stream;
    bool          failed;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "text";

    case TREE_RENDER:
        // Dump the text as a string of characters, doubling quotes
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

        text = (text_p) tree;
        data = text_data(text);
        size = text_size(text);
        failed = io(stream, 1, "\"") != 1;
        while (size--)
        {
            char c = *data++;
            if (c == '"')
                failed |= io(stream, 2, "\"\"") != 2;
            else
                failed |= io(stream, 1, &c) != 1;
        }
        failed |= io(stream, 1, "\"") != 1;
        return failed ? NULL : tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}
