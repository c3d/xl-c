// ****************************************************************************
//  name.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of name in the XL compiler
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

#include "name.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>



bool name_is_operator(name_p name)
// ----------------------------------------------------------------------------
//   Return true if the name is an operator (e.g. + or -=)
// ----------------------------------------------------------------------------
{
    return ispunct(*name_data(name));
}


bool name_is_valid(size_t size, const char *data)
// ----------------------------------------------------------------------------
//    Check if a name obeys the syntax rules
// ----------------------------------------------------------------------------
{
    if (size)
    {
        if (ispunct(*data))
        {
            while (size--)
            {
                if (!ispunct(*data++))
                    return false;
            }
            return true;
        }
        else if (isalpha(*data))
        {
            bool had_underscore = true;
            while (size--)
            {
                char c = *data++;
                if (c == '_' && had_underscore)
                    return false;
                had_underscore = c == '_';
                if (!had_underscore && !isalnum(c))
                    return false;
            }
            return true;
        }
    }
    return false;
}


tree_p name_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for names deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    name_r        name = (name_r) tree;
    size_t        size;
    tree_io_fn    io;
    const char  * data;
    void *        stream;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "name";

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see name_make)
        size = va_arg(va, size_t);
        data = va_arg(va, const char *);
        assert(name_is_valid(size, data) && "Name must respect XL syntax");

        // Create name and copy data in it
        name = (name_r) tree_malloc(sizeof(name_t) + size);
        memcpy(name + 1, data, size);
        name->text.blob.length = size;
        return (tree_p) name;

    case TREE_RENDER:
        // Dump the name as a string of characters, doubling quotes
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

        name = (name_r) tree;
        data = name_data(name);
        size = name_length(name);
        if (io(stream, size, (void *) data) != size)
            return NULL;
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}
