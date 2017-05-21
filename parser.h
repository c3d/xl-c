#ifndef PARSER_H
#define PARSER_H
// ****************************************************************************
//  parser.h                                        XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     XL parser
//
//
//
//
//
//
//
//
// ****************************************************************************
//  (C) 1992-2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************
/*
  Parsing XL is extremely simple. The source code is transformed into
  a tree with only three type of nodes and four types of leafs:

  The three node types are:
  - Prefix operator as "not" in "not A" or "+" in "+7"
  - Infix operator as "-" in "A-B" or "and" in "3 and 5"
  - Parenthese grouping as in "(A+B)" or "[D + E]".

  The four leaf types are:
  - Integer numbers such as 130 or 16#FE
  - Real numbers such as 0.1 or 10.4E-31
  - Text such as "Hello" or 'ABC'
  - Name/symbols such as ABC or --->

  High-level program structure is also represented by these same nodes:
  - A sequence of statements on a same line is a semi-colon infix-op:
        Do; Redo
  - A sequence of statements on multiple line is a "new-line" infix-op:
        Do
        Redo
  - A sequence of parameters is made of "comma" infix-op, and a statement
    is a prefix-op with this sequence of parameters as argument:
        WriteLn A,B
  - By default, a sequence of tokens is parsed using prefix operator,
    unless a token is recognized as an infix operator.
        A and B or C
    parses by default as
        A(and(B(or(C))))
    but if 'and' and 'or' are declared as infix operators, it parses as
        ((A and B) or C)
    or
        (A and (B or C))
    depending on the relative precedences of 'and' and 'or'.

  With this scheme, only infix operators need to be declared. In some
  contexts, a name declared as being an infix operator still parses prefix,
  for instance in (-A-B), where the first minus is a prefix.
  Any name or symbol is valid to identify a prefix or infix operator.

  Operator precedence is defined by the elfe.syntax file.

  Comments and extraneous line separators are preserved in CommentsInfo nodes
  attached to the returned parse trees.
*/

#include "error.h"
#include "scanner.h"
#include "syntax.h"
#include "tree.h"



// ============================================================================
//
//    The parser itself
//
// ============================================================================

typedef struct parser
// ----------------------------------------------------------------------------
//   Information needed to parse an input file and return an XL parse tree
// ----------------------------------------------------------------------------
{
    scanner_p   scanner;
    text_p      comment;
    token_t     pending;
    bool        had_space_before : 1;
    bool        had_space_after  : 1;
    bool        beginning_line   : 1;
} parser_t, *parser_p;


extern parser_p parser_new(const char *filename, positions_p, syntax_p);
extern void     parser_delete(parser_p p);
extern tree_p   parser_parse(parser_p p);

#endif // PARSER_H
