#ifndef DELIMITED_TEXT_H
#define DELIMITED_TEXT_H
// ****************************************************************************
//  delimited_text.h                                XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Text with specific delimiters, e.g. << Hello >>
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

#include "text.h"
#include "name.h"

typedef struct delimited_text
// ----------------------------------------------------------------------------
//   Text with delimiters
// ----------------------------------------------------------------------------
{
    tree_t tree;
    text_p value;
    name_p opening;
    name_p closing;
} delimited_text_t;

#ifdef DELIMITED_TEXT_C
#define inline extern inline
#endif

tree_type(delimited_text);
inline delimited_text_r delimited_text_new(srcpos_t position, text_p value,
                                           name_p opening, name_p closing);
inline delimited_text_r delimited_text_make(tree_handler_fn h,
                                            srcpos_t position, text_p value,
                                            name_p opening, name_p closing);
extern tree_p dellmited_text_handler(tree_cmd_t cmd, tree_p tree, va_list va);

#undef inline


// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline delimited_text_r delimited_text_make(tree_handler_fn h,
                                            srcpos_t position, text_p value,
                                            name_p opening, name_p closing)
// ----------------------------------------------------------------------------
//   Make a delimited text with a specific handler
// ----------------------------------------------------------------------------
{
    return (delimited_text_r) tree_make(h, position, value, opening, closing);
}

inline delimited_text_r delimited_text_new(srcpos_t position, text_p value,
                                           name_p opening, name_p closing)
// ----------------------------------------------------------------------------
//   Build new delimited text
// ----------------------------------------------------------------------------
{
    return delimited_text_make(delimited_text_handler,
                               position, value, opening, closing);
}

#endif // DELIMITED_TEXT_H
