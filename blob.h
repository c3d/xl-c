#ifndef BLOB_H
#define BLOB_H
// ****************************************************************************
//  blob.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Blobs (short for "binary lumped object") are used to store
//     binary data that is not otherwise interpreted by the XL language
//     in any way. The most common example is text. A blob is a sized
//     sequence of bytes in memory, and can contain anything. What the data
//     actually means is left to the handler.
//
//
//
//
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "tree.h"
#include <stdint.h>


typedef struct blob
// ----------------------------------------------------------------------------
//   A blob internal representation is little more than a stream of bytes
// ----------------------------------------------------------------------------
//   The bytes are allocated immediately after the blob
{
    tree_t      tree;           // The base tree
    size_t      size;           // Size in bytes of the data that follows
} blob_t;


inline blob_p      blob_new(unsigned position, size_t sz, const char *data);
inline void        blob_delete(blob_p blob);
extern blob_p      blob_append(blob_p blob, size_t sz, const char *data);
inline const char *blob_data(blob_p blob);
inline size_t      blob_size(blob_p blob);

// Private blob handler, should not be called directly in general
inline blob_p blob_make(tree_handler_fn h, unsigned pos, size_t, const char *);
extern tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va);



// ============================================================================
// 
//   Inline implementations
// 
// ============================================================================

inline blob_p blob_make(tree_handler_fn h, unsigned pos,
                        size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Create a blob with the given parameters
// ----------------------------------------------------------------------------
{
    return (blob_p) tree_make(h, pos, sz, data);
}


inline blob_p blob_new(unsigned position, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//    Allocate a blob with the given data
// ----------------------------------------------------------------------------
{
    return blob_make(blob_handler, position, sz, data);
}


inline void blob_delete(blob_p blob)
// ----------------------------------------------------------------------------
//   Delete the given blob
// ----------------------------------------------------------------------------
{
    tree_delete((tree_p) blob);
}


inline const char *blob_data(blob_p blob)
// ----------------------------------------------------------------------------
//   Return the data for the blob
// ----------------------------------------------------------------------------
{
    return (const char *) (blob + 1);
}


inline size_t blob_size(blob_p blob)
// ----------------------------------------------------------------------------
//   Return the data for the blob
// ----------------------------------------------------------------------------
{
    return blob->size;
}


#endif // BLOB_H
