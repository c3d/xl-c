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


blob_p blob_append(blob_p blob, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append data to the blob - In place if possible
// ----------------------------------------------------------------------------
//   We can append in place if:
//   - There is only one user of this blob (who, presumably, is calling us)
//   - The realloc can extend memory without copy
{
    blob_p copy = blob;
    if (blob_ref(blob) > 1)
        copy = NULL;
    blob_p result = (blob_p) realloc(copy, blob->size + sz);
    if (result)
    {
        if (copy == NULL)
            memcpy(result, blob, sizeof(blob_t) + blob->size);
        char *append_dst = (char *) result + sizeof(blob_t) + blob->size;
        memcpy(append_dst, data, sz);
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
    blob_p copy = blob;
    unsigned end = first + length;
    if (end > blob->size)
        end = blob->size;
    if (first > blob->size)
        first = blob->size;
    unsigned resized = end - first;
    if (blob_ref(blob) > 1)
    {
        copy = (blob_p) malloc(sizeof(blob_t) + resized);
        memcpy(copy, blob, sizeof(blob_t));
    }
    memmove(copy + 1, blob + 1, resized);
    if (copy == blob)
        copy = (blob_p) realloc(copy, sizeof(blob_t) + resized);
    blob_unref(blob);
    return copy;
}


tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   The handler for blobs deals mostly with variable-sized initialization
// ----------------------------------------------------------------------------
{
    blob_p        blob = (blob_p) tree;
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
        return (tree_p) (sizeof(blob_t) + blob->size);

    case TREE_ARITY:
        // The arity for blobs is normally 0
        return (tree_p) 0;

    case TREE_INITIALIZE:
        // Fetch pointer to data and size from varargs list (see blob_new)
        size = va_arg(va, size_t);
        data = va_arg(va, const char *);

        // Create blob and copy data in it
        blob = (blob_p) malloc(sizeof(blob_t) + size);
        if (size)
            memcpy(blob + 1, data, size);
        return (tree_p) blob;

    case TREE_RENDER:
        // Dump the blob as an hexadecimal string
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

        size = snprintf(buffer, sizeof(buffer),
                        "%s %zu ",
                        blob_typename(blob), blob->size);
        if (io(stream, size, buffer) != size)
            return NULL;        // Some error happened, report
        data = (const char *) (blob + 1);
        for (idx = 0; idx < blob->size;idx++)
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
