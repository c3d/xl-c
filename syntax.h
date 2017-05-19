#ifndef SYNTAX_H
#define SYNTAX_H
// ****************************************************************************
//  syntax.h                                        XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Internal description of an XL syntax file
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

#include "name.h"
#include "blob.h"
#include "array.h"


#ifdef SYNTAX_C
#define inline extern inline
#endif // SYNTAX_C

struct syntax;
#define syntaxes_handler blob_handler
blob_type(struct syntax *, syntaxes);

#undef inline


typedef struct syntax
// ----------------------------------------------------------------------------
//   Internal description of the syntax configuration in xl.syntax
// ----------------------------------------------------------------------------
{
    // File name we read the syntax from
    text_p              filename;

    // Priorities: sorted array of pairs (name, priority)
    array_p             infix;
    array_p             prefix;
    array_p             postfix;

    // Delimiters: array of pairs (opening, closing)
    array_p             comment;
    array_p             text;
    array_p             block;

    // Delimiters for child syntax, and the table itself
    array_p             syntax;
    syntaxes_p          syntaxes;

    // Priorities
    int                 default_priority;
    int                 statement_priority;
    int                 function_priority;
} syntax_t, *syntax_p;


// Create and delete syntax files
extern syntax_p syntax_new(const char *file);
extern void     syntax_delete(syntax_p s);


// Checking syntax elements
extern bool syntax_is_operator(syntax_p, text_p name);
extern bool syntax_is_block_open(syntax_p, name_p name);
extern bool syntax_is_block_close(syntax_p, name_p name);
extern bool syntax_is_block_open_character(syntax_p, char name);
extern bool syntax_is_block_close_character(syntax_p, char name);

#endif // SYNTAX_H
