// ****************************************************************************
//  blob.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Implementation of binary lumped objects
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

#define BLOB_C
#include "blob.h"

#include "renderer.h"

#include <stdlib.h>
#include <string.h>


void blob_append_data(blob_p *blob_ptr, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append data to the blob - In place if possible
// ----------------------------------------------------------------------------
//   We can append in place if:
//   - There is only one user of this blob (who, presumably, is calling us)
//   - The realloc can extend memory without copy
{
    blob_p blob = *blob_ptr;
    blob_p in_place = blob;
    if (blob_ref(blob))
        in_place = NULL;
    size_t old_size = sizeof(blob_t) + blob->length;
    size_t new_size = old_size + sz;
    blob_p result = (blob_p) tree_realloc((tree_p) in_place, new_size);
    if (result)
    {
        if (!in_place)
        {
            memcpy(result, blob, old_size);
            result->tree.refcount = 0;
        }
        char *append_dst = blob_data(result) + blob_length(blob);
        if (data)
            memcpy(append_dst, data, sz);
        else
            memset(append_dst, 0, sz);
        result->length += sz;
    }
    blob_unref(blob);
    if (result != blob)
    {
        if (in_place)
            *blob_ptr = result;
        else
            blob_set(blob_ptr, result);
    }
}


void blob_range(blob_p *blob_ptr, size_t first, size_t length)
// ----------------------------------------------------------------------------
//   Select a range of the blob, in place if possible
// ----------------------------------------------------------------------------
//   We can move in place if there is only one user of this blob
{
    blob_p blob = *blob_ptr;
    blob_p in_place = blob;
    size_t end = first + length;
    if (end > blob->length)
        end = blob->length;
    if (first > blob->length)
        first = blob->length;
    size_t resized = end - first;
    if (blob_ref(blob))
    {
        in_place = (blob_p) tree_malloc(sizeof(blob_t) + resized);
        memcpy(in_place, blob, sizeof(blob_t));
        in_place->tree.refcount = 0;
    }
    memmove(in_place + 1, blob_data(blob) + first, resized);
    in_place->length = resized;
    if (in_place == blob)
        in_place = (blob_p) tree_realloc((tree_p) in_place, sizeof(blob_t) + resized);
    blob_unref(blob);

    if (in_place != blob)
        blob_set(blob_ptr, in_place);
}


int blob_compare(blob_p b1, blob_p b2)
// ----------------------------------------------------------------------------
//   Compare two blobs (lexical order)
// ----------------------------------------------------------------------------
{
    char *  p1  = blob_data(b1);
    char *  p2  = blob_data(b2);
    size_t  l1  = blob_length(b1);
    size_t  l2  = blob_length(b2);
    size_t  l   = l1 < l2 ? l1 : l2;
    int     cmp = memcmp(p1, p2, l);
    if (cmp == 0)
    {
        if (l1 < l2)
            cmp = -1;
        else if (l1 > l2)
            cmp = 1;
    }
    return cmp;
}


tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blobs deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    blob_p        blob = (blob_p) tree;
    size_t        size, idx;
    renderer_p    renderer;
    const char  * data;
    char          buffer[32];

    switch(cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "blob";

    case TREE_SIZE:
        // Return the size of the tree in bytes (is dynamic for blobs)
        return (tree_p) (sizeof(blob_t) + blob->length);

    case TREE_ARITY:
        // The arity for blobs is normally 0
        return (tree_p) 0;

    case TREE_CAST:
        // Check if we cast to blob type, if so, success
        if (tree_cast_handler(va) == blob_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see blob_new)
        size = va_arg(va, size_t);
        data = va_arg(va, const char *);

        // Create blob and copy data in it
        blob = (blob_p) tree_malloc(sizeof(blob_t) + size);
        blob->length = size;
        if (size)
        {
            if (data)
                memcpy(blob + 1, data, size);
            else
                memset(blob + 1, 0, size);
        }
        return (tree_p) blob;

    case TREE_RENDER:
        // Dump the blob as an hexadecimal string
        renderer = va_arg(va, renderer_p);
        render_text(renderer, 1, "$");
        data = (const char *) (blob + 1);
        for (idx = 0; idx < blob->length;idx++)
        {
            size = snprintf(buffer, sizeof(buffer), "%02X", data[idx]);
            render_text(renderer, size, buffer);
        }
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        break;
    }
    return tree_handler(cmd, tree, va);
}
