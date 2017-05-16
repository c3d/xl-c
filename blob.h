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
    size_t      length;         // Size in bytes of the data that follows
} blob_t;
tree_typedef(blob);


inline blob_r  blob_new(unsigned position, size_t sz, const char *data);
extern blob_p  blob_append_data(blob_p blob, size_t sz, const char *data);
inline blob_p  blob_append(blob_p blob, blob_p other);
extern blob_p  blob_range(blob_p blob, size_t start, size_t len);
inline char   *blob_data(blob_p blob);
inline size_t  blob_length(blob_p blob);

// Private blob handler, should not be called directly in general
inline blob_r blob_make(tree_handler_fn h, unsigned pos, size_t, const char *);
extern tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va);



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline blob_r blob_make(tree_handler_fn h, unsigned pos,
                        size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Create a blob with the given parameters
// ----------------------------------------------------------------------------
{
    return (blob_r) tree_make(h, pos, sz, data);
}


inline blob_r blob_new(unsigned position, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//    Allocate a blob with the given data
// ----------------------------------------------------------------------------
{
    return blob_make(blob_handler, position, sz, data);
}


inline blob_p blob_append(blob_p blob, blob_p blob2)
// ----------------------------------------------------------------------------
//   Append one blob to another
// ----------------------------------------------------------------------------
{
    return blob_append_data(blob, blob_length(blob2), blob_data(blob2));
}


inline char *blob_data(blob_p blob)
// ----------------------------------------------------------------------------
//   Return the data for the blob
// ----------------------------------------------------------------------------
{
    return (char *) (blob + 1);
}


inline size_t blob_length(blob_p blob)
// ----------------------------------------------------------------------------
//   Return the data for the blob
// ----------------------------------------------------------------------------
{
    return blob->length;
}



// ============================================================================
//
//    Blobs for a given item type
//
// ============================================================================

#define blob_typedef(item, name)                                        \
                                                                        \
typedef const struct name *name##_p;                                    \
typedef       struct name *name##_r;                                    \
                                                                        \
inline name##_r *name##_new(unsigned pos, size_t sz, const item *data)  \
{                                                                       \
    sz *= sizeof(item);                                                 \
    const char *chardata = (const char *) data;                         \
    return (name##_r) blob_new(pos, sz, chardata);                      \
}                                                                       \
                                                                        \
inline void name##_delete(name##_p name)                                \
{                                                                       \
    blob_delete((blob_p) name)                                          \
}                                                                       \
                                                                        \
inline name##_p name##_append(name##_p name, name##_p name2)            \
{                                                                       \
    return (name##_p) blob_append((blob_p) name, (blob_p) name2);       \
}                                                                       \
                                                                        \
                                                                        \
inline name##_p name##_append_data(name##_p name,                       \
                                   size_t sz, const item *data)         \
{                                                                       \
    sz *= sizeof(item);                                                 \
    const char *chardata = (const char *) data;                         \
    return (name##_p) blob_append_data((blob_p) name, sz, chardata);    \
}                                                                       \
                                                                        \
                                                                        \
inline name##_p name##_range(name##_p name, size_t start, size_t len)   \
{                                                                       \
    start *= sizeof(item);                                              \
    len *= sizeof(item);                                                \
    return (name##_p) blob_range((blob_p) name, start, len);            \
}                                                                       \
                                                                        \
inline item *name##_data(name##_p name)                                 \
{                                                                       \
    return (item *) blob_data((blob_p) name);                           \
}                                                                       \
                                                                        \
inline size_t name##_length(name##_p blob)                              \
{                                                                       \
    return blob_length((blob_p) name) / sizeof(item);                   \
}                                                                       \
                                                                        \
inline name##_p name##_push(name##_p name, item value)                  \
{                                                                       \
    return (name##_p) blob_append((blob_p) name,                        \
                                  sizeof(value), &value));              \
}                                                                       \
                                                                        \
inline item name##_top(name##_p name)                                   \
{                                                                       \
    return name##_data(name)[name##_length(name)-1];                    \
}                                                                       \
                                                                        \
inline name##_p name##_pop(name##_p name)                               \
{                                                                       \
    assert(name##_length(name) && "Can only pop if non-empty");         \
    return name##_range(name, 0, name_length(name)-1);                  \
}



#endif // BLOB_H
