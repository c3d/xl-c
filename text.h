#ifndef TEXT_H
#define TEXT_H
// ****************************************************************************
//  text.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of text in the XL compiler
//
//     Text nodes are used both to represent textual data in the source,
//     for example "Hello world", and run-time text during execution.
//     They are mostly implemented as a blob and as a result may contain
//     NUL characters.
//
//
//
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "blob.h"

typedef struct text
// ----------------------------------------------------------------------------
//   A blob internal representation is little more than a stream of bytes
// ----------------------------------------------------------------------------
//   The bytes are allocated immediately after the blob
{
    blob_t      blob;           // The base blob
} text_t;


inline text_p      text_new(unsigned position, size_t sz, const char *data);
inline void        text_delete(text_p text);
inline text_p      text_append(text_p text, size_t sz, const char *data);
inline const char *text_data(text_p text);
inline size_t      text_size(text_p text);

// Private text handler, should not be called directly in general
extern text_p text_make(tree_handler_fn h, unsigned pos, size_t, const char *);
extern tree_p text_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Helper macro to initialize with a C constant
#define text_cnew(pos, text)    text_new(pos, sizeof(text), text)



// ============================================================================
// 
//   Inline implementations
// 
// ============================================================================

inline text_p text_new(unsigned position, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//    Allocate a text with the given data
// ----------------------------------------------------------------------------
{
    return text_make(text_handler, position, sz, data);
}


inline void text_delete(text_p text)
// ----------------------------------------------------------------------------
//   Delete the given text
// ----------------------------------------------------------------------------
{
    tree_delete((tree_p) text);
}


inline text_p text_append(text_p text, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append text, possibly in place
// ----------------------------------------------------------------------------
{
    return (text_p) blob_append((blob_p) text, sz, data);
}


inline const char *text_data(text_p text)
// ----------------------------------------------------------------------------
//   Return the data for the text
// ----------------------------------------------------------------------------
{
    return (const char *) (text + 1);
}


inline size_t text_size(text_p text)
// ----------------------------------------------------------------------------
//   Return the data for the text
// ----------------------------------------------------------------------------
{
    return text->blob.size;
}

#endif // TEXT_H
