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


text_p text_make(tree_handler_fn h, unsigned pos, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Create a text with the given parameters
// ----------------------------------------------------------------------------
{
    return (text_p) tree_make(h, pos, sz, data);
}


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

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for texts)
        return (tree_p) (sizeof(text_t) + text->blob.size);

    case TREE_ARITY:
        // The arity for texts is 0
        return (tree_p) 0;

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see text_make)
        size = va_arg(va, size_t);
        data = va_arg(va, const char *);

        // Create text and copy data in it
        text = (text_p) malloc(sizeof(text_t) + size);
        memcpy(text + 1, data, size);
        return (tree_p) text;

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
