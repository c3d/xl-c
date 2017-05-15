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
#include <stdarg.h>


typedef struct text
// ----------------------------------------------------------------------------
//   A text internal representation is little more than a stream of bytes
// ----------------------------------------------------------------------------
//   The bytes are allocated immediately after the text_t structure
{
    blob_t      blob;           // The base blob
} text_t;
tree_typedef(text);


inline text_r      text_new(unsigned position, size_t sz, const char *data);
inline text_p      text_append_data(text_p text, size_t sz, const char *data);
inline text_p      text_append(text_p text, text_p text2);
inline text_p      text_range(text_p text, size_t start, size_t length);
inline const char *text_data(text_p text);
inline size_t      text_length(text_p text);
extern text_r      text_printf(unsigned pos, const char *format, ...);
extern text_r      text_vprintf(unsigned pos, const char *format, va_list va);

// Private text handler, should not be called directly in general
inline text_r text_make(tree_handler_fn h, unsigned pos, size_t, const char *);
extern tree_p text_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Helper macro to initialize with a C constant
#define text_cnew(pos, text)    text_new(pos, sizeof(text), text)



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline text_r text_make(tree_handler_fn h, unsigned pos,
                        size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Create a text with the given parameters
// ----------------------------------------------------------------------------
{
    return (text_r) tree_make(h, pos, sz, data);
}


inline text_r text_new(unsigned position, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//    Allocate a text with the given data
// ----------------------------------------------------------------------------
{
    return text_make(text_handler, position, sz, data);
}


inline text_p text_append_data(text_p text, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append text, possibly in place
// ----------------------------------------------------------------------------
{
    return (text_p) blob_append_data((blob_p) text, sz, data);
}


inline text_p text_append(text_p text, text_p text2)
// ----------------------------------------------------------------------------
//   Append text, possibly in place
// ----------------------------------------------------------------------------
{
    return (text_p) blob_append((blob_p) text, (blob_p) text2);
}


inline text_p text_range(text_p text, size_t start, size_t length)
// ----------------------------------------------------------------------------
//   Extract a range of text, ideally in place
// ----------------------------------------------------------------------------
{
    return (text_p) blob_range((blob_p) text, start, length);
}


inline const char *text_data(text_p text)
// ----------------------------------------------------------------------------
//   Return the data for the text
// ----------------------------------------------------------------------------
{
    return (const char *) (text + 1);
}


inline size_t text_length(text_p text)
// ----------------------------------------------------------------------------
//   Return the data for the text
// ----------------------------------------------------------------------------
{
    return text->blob.size;
}

#endif // TEXT_H
