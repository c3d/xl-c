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

#ifdef BLOB_C
#define inline extern inline
#endif

tree_type(blob);
inline blob_r  blob_new(srcpos_t position, size_t sz, const char *data);
extern blob_p  blob_append_data(blob_p blob, size_t sz, const char *data);
inline blob_p  blob_append(blob_p blob, blob_p other);
extern blob_p  blob_range(blob_p blob, size_t start, size_t len);
inline char   *blob_data(blob_p blob);
inline size_t  blob_length(blob_p blob);

// Private blob handler, should not be called directly in general
inline blob_r blob_make(tree_handler_fn h, srcpos_t pos, size_t, const char *);
extern tree_p blob_handler(tree_cmd_t cmd, tree_p tree, va_list va);

#undef inline



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline blob_r blob_make(tree_handler_fn h, srcpos_t pos,
                        size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Create a blob with the given parameters
// ----------------------------------------------------------------------------
{
    return (blob_r) tree_make(h, pos, sz, data);
}


inline blob_r blob_new(srcpos_t position, size_t sz, const char *data)
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

#define blob_type(item, type)                                           \
                                                                        \
    tree_type(type);                                                    \
                                                                        \
    inline type##_r type##_new(srcpos_t pos, size_t sz, const item *data) \
    {                                                                   \
        sz *= sizeof(item);                                             \
        const char *chardata = (const char *) data;                     \
        return (type##_r) blob_new(pos, sz, chardata);                  \
    }                                                                   \
                                                                        \
    inline type##_p type##_append(type##_p type, type##_p type2)        \
    {                                                                   \
        return (type##_p) blob_append((blob_p) type, (blob_p) type2);   \
    }                                                                   \
                                                                        \
                                                                        \
    inline type##_p type##_append_data(type##_p type,                   \
                                       size_t sz, const item *data)     \
    {                                                                   \
        sz *= sizeof(item);                                             \
        const char *chardata = (const char *) data;                     \
        return (type##_p) blob_append_data((blob_p) type, sz, chardata); \
    }                                                                   \
                                                                        \
                                                                        \
    inline type##_p type##_range(type##_p type, size_t start, size_t len) \
    {                                                                   \
        start *= sizeof(item);                                          \
        len *= sizeof(item);                                            \
        return (type##_p) blob_range((blob_p) type, start, len);        \
    }                                                                   \
                                                                        \
    inline item *type##_data(type##_p type)                             \
    {                                                                   \
        return (item *) blob_data((blob_p) type);                       \
    }                                                                   \
                                                                        \
    inline size_t type##_length(type##_p type)                          \
    {                                                                   \
        return blob_length((blob_p) type) / sizeof(item);               \
    }                                                                   \
                                                                        \
    inline type##_p type##_push(type##_p type, item value)              \
    {                                                                   \
        return (type##_p) blob_append_data((blob_p) type,               \
                                           sizeof(value),               \
                                           (const char *) &value);      \
    }                                                                   \
                                                                        \
    inline item type##_top(type##_p type)                               \
    {                                                                   \
        return type##_data(type)[type##_length(type)-1];                \
    }                                                                   \
                                                                        \
    inline type##_p type##_pop(type##_p type)                           \
    {                                                                   \
        assert(type##_length(type) && "Can only pop if non-empty");     \
        size_t new_size = sizeof(item) * (type##_length(type) - 1);     \
        return type##_range(type, 0, new_size);                         \
    }

#endif // BLOB_H
