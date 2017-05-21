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

tree_type(syntax);

#undef inline

typedef struct syntax
// ----------------------------------------------------------------------------
//   Internal description of the syntax configuration in xl.syntax
// ----------------------------------------------------------------------------
{
    tree_t              tree;

    // File name we read the syntax from
    text_p              filename;

    // Known operators
    array_p             known;

    // Priorities: sorted array of pairs (name, priority)
    array_p             infixes;
    array_p             prefixes;
    array_p             postfixes;

    // Delimiters: array of pairs (opening, closing)
    array_p             comments;
    array_p             texts;
    array_p             blocks;

    // Delimiters for child syntax, and the table itself
    array_p             syntaxes;

    // Priorities
    int                 default_priority;
    int                 statement_priority;
    int                 function_priority;
} syntax_t;

// Forward declaration
typedef struct scanner *scanner_p;


// Create and delete syntax files
extern syntax_p syntax_new(const char *file);
extern void     syntax_read_file(syntax_r syntax, const char *file);
extern void     syntax_read(syntax_r syntax, scanner_p scanner);
extern tree_p   syntax_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Checking syntax elements
extern int      syntax_infix_priority(syntax_p, name_p name);
extern int      syntax_prefix_priority(syntax_p, name_p name);
extern int      syntax_postfix_priority(syntax_p, name_p name);
extern bool     syntax_is_operator(syntax_p, name_p name);
extern bool     syntax_is_block(syntax_p, name_p name, name_p *closing);
extern bool     syntax_is_text(syntax_p, name_p name, name_p *closing);
extern bool     syntax_is_comment(syntax_p, name_p name, name_p *closing);
extern syntax_p syntax_is_special(syntax_p, name_p name, name_p *closing);

// Internal representation of block indent and unindent
#define SYNTAX_INDENT    "\t"
#define SYNTAX_UNINDENT  "\b"

#endif // SYNTAX_H
