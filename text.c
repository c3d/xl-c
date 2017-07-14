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

#define TEXT_C
#include "text.h"

#include "renderer.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

tree_p text_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for texts deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    text_p        text = (text_p) tree;
    size_t        size;
    const char  * data;
    renderer_p    renderer;

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "text";

    case TREE_CAST:
        // Check if we cast to blob type, if so, success
        if (tree_cast_handler(va) == text_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_RENDER:
        // Dump the text as a string of characters, doubling quotes
        renderer = va_arg(va, renderer_p);

        text = (text_p) tree;
        data = text_data(text);
        size = text_length(text);
        render_open_quote(renderer, '"');
        render_text(renderer, size, data);
        render_close_quote(renderer, '"');
        return tree;

    default:
        break;
    }
    // Other cases are handled correctly by the blob handler
    return blob_handler(cmd, tree, va);
}


text_p text_printf(srcpos_t pos, const char *format, ...)
// ----------------------------------------------------------------------------
//    Format input with printf-like style and %t extension for trees
// ----------------------------------------------------------------------------
//    The implementation requires that all %t parameters come first
{
    va_list va;
    va_start(va, format);
    text_p result = text_vprintf(pos, format, va);
    va_end(va);
    return result;
}


extern text_p text_vprintf(srcpos_t pos, const char *format, va_list va)
// ----------------------------------------------------------------------------
//   Format input with printf-style format, and %t extension for trees
// ----------------------------------------------------------------------------
//   The implementation requires that all %t parameters come first
//   and all % formats before the first %t are ignored.
{
    unsigned size = strlen(format) + 1; // +1 for trailing 0
    text_p result = text_new(pos, size, format);

    char *base = (char *) text_data(result);
    const char *search = base;

    // First pass: replace all %t with result of tree_render
    while(true)
    {
        // Find the next '%t' in the format and compute offset in result
        char *tpos = strstr(search, "%t");
        if (!tpos)
            break;
        unsigned offset = tpos - base;

        // Read input tree from varargs and turn it to text format
        tree_p tree = va_arg(va, tree_p);
        text_p ins = tree_text(tree);

        // Insertion data and size
        const char *ins_data = text_data(ins);
        unsigned ins_size = text_length(ins);

        // Compute size after insertion and extend result as necessary
        unsigned old_size = text_length(result);
        unsigned new_size = old_size + ins_size - 2;
        if (new_size > old_size)
            text_append_data(&result, ins_size - 2, NULL);

        // Move the text following %t up
        char *data = (char *) text_data(result);
        unsigned mov_size = new_size - offset - 2;
        memmove(data + offset + ins_size, data + offset + 2, mov_size);

        // Copy the insertion text in the middle
        memcpy(data + offset, ins_data, ins_size);

        // Truncate the result if the insertion is less than 2 bytes long
        if (new_size < old_size)
            text_range(&result, 0, new_size);

        // Dispose of inserted text
        text_dispose(&ins);

        // Prepare for next loop. Place search after what we inserted.
        base = (char *) text_data(result);
        search = base + offset + ins_size;
    }

    // At that point, result contains a nul-terminated format without %t
    // We use the format starting at 'search' because the trees may
    // themselves have inserted for example %s.
    size = text_length(result) - 1;
    unsigned offset = search - base;
    text_append_data(&result, 1024 + size, NULL);
    base = (char *) text_data(result);
    format = base + offset;
    base[size] = 0;             // Make sure format is null-terminated
    unsigned act_size = vsnprintf(base + size + 1, 1023 + size, format, va);

    // Truncate result to actual size
    memmove((char *) format, base + size + 1, act_size);
    text_range(&result, 0, offset + act_size);

    return result;
}
