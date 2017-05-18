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

#ifdef TEXT_C
#define inline extern inline
#endif

blob_type(char, text);
extern text_r      text_printf(srcpos_t pos, const char *format, ...);
extern text_r      text_vprintf(srcpos_t pos, const char *format, va_list va);

// Private text handler, should not be called directly in general
inline text_r text_make(tree_handler_fn h, srcpos_t pos, size_t, const char *);
extern tree_p text_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Helper macro to initialize with a C constant
#define text_cnew(pos, text)    text_new(pos, strlen(text), text)

#undef inline

#endif // TEXT_H
