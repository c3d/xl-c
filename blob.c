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

#include "blob.h"
#include <stdlib.h>
#include <string.h>


blob_p blob_append_data(blob_p blob, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append data to the blob - In place if possible
// ----------------------------------------------------------------------------
//   We can append in place if:
//   - There is only one user of this blob (who, presumably, is calling us)
//   - The realloc can extend memory without copy
{
    blob_r copy = (blob_r) blob;
    if (blob_ref(blob))
        copy = NULL;
    size_t old_size = sizeof(blob_t) + blob->length;
    size_t new_size = old_size + sz;
    blob_r result = (blob_r) tree_realloc((tree_r) copy, new_size);
    if (result)
    {
        if (result != copy)
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
    return result;
}


blob_p blob_range(blob_p blob, size_t first, size_t length)
// ----------------------------------------------------------------------------
//   Select a range of the blob, in place if possible
// ----------------------------------------------------------------------------
//   We can move in place if there is only one user of this blob
{
    blob_r copy = (blob_r) blob;
    size_t end = first + length;
    if (end > blob->length)
        end = blob->length;
    if (first > blob->length)
        first = blob->length;
    size_t resized = end - first;
    if (blob_ref(blob))
    {
        copy = (blob_r) tree_malloc(sizeof(blob_t) + resized);
        memcpy(copy, blob, sizeof(blob_t));
        copy->tree.refcount = 0;
    }
    memmove(copy + 1, blob_data(blob) + first, resized);
    copy->length = resized;
    if (copy == blob)
        copy = (blob_r) tree_realloc((tree_r) copy, sizeof(blob_t) + resized);
    blob_unref(blob);
    return copy;
}


tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blobs deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    blob_r        blob = (blob_r) tree;
    size_t        size, idx;
    tree_io_fn    io;
    const char  * data;
    void *        stream;
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

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see blob_new)
        size = va_arg(va, size_t);
        data = va_arg(va, const char *);

        // Create blob and copy data in it
        blob = (blob_r) tree_malloc(sizeof(blob_t) + size);
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
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

        if (io(stream, 1, "$") != 1)
            return NULL;        // Some error happened, report
        data = (const char *) (blob + 1);
        for (idx = 0; idx < blob->length;idx++)
        {
            size = snprintf(buffer, sizeof(buffer), "%02X", data[idx]);
            if (io(stream, size, buffer) != size)
                return NULL;
        }
        return tree;

    default:
        // Other cases are handled correctly by the tree handler
        return tree_handler(cmd, tree, va);
    }
}
