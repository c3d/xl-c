#ifndef SCANNER_H
#define SCANNER_H
// ****************************************************************************
//  scanner.h                                       XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Interface for the XL scanner
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
  XL scanning is quite simple. There are only five types of built-in tokens:

  - Integer or real numbers, beginning with a digit
  - Names, beginning with a letter
  - Text, enclosed in single or double quotes
  - Symbols, formed by consecutive sequences of punctuation characters
  - Blanks and line separators


  NUMBERS:

  Numbers begin with a digit (i.e. .3 is not a valid number, 0.3 is)
  Numbers can be written in any base between 2 and 36, using the '#'
  notation: 16#FF. In that case, letters A-Z represent digit values 10..35.
  In addition, the base can be set to 64, in which case the number
  uses the base-64 encoding (this is mostly useful for blobs)
  They can contain a decimal dot to specify real numbers: 5.21
  They can contain single underscores to group digits: 1_980_000
  They can contain an exponent introduced with the letter E: 1.31E6
  The exponent can be negative, indicating a real number: 1.31E-6; 1E-3
  Another '#' sign can be used before 'E', in particular when 'E' is
  a digit of the base: 16#FF#E20
  The exponent represents a power of the base: 16#FF#E2 is 16#FF00
  Combinations of the above are valid: 16#FF_00.00_FF#E-5

  NAMES:

  Names begin with any letter, and are made of letters or digits: R19, Hello
  Names can contain single underscores to group words: Big_Number
  Names are not case-sensitive nor underscore-sensitive: Joe_Dalton=JOEDALTON

  SYMBOLS:

  Symbols begin with any punctuation character except quotes, and contain
  the largest sequence of such punctuation characters that is recorded as a
  valid symbol in the syntax table. If the symbol is not defined in
  the syntax table, then only one character is used.

  TEXT AND CHARACTERS:

  Text begins and ends with a single or a double quote. It can contain
  practically any character, including line-ending characters. The
  quote character used to begin the text can be embedded in text by
  doubling it. Later stages will treat 'C' as a character and "C" as
  text, which explains why different tokens are returned. But the
  scanner allows characters to contain multi-byte sequences or
  special encodings.

  The scanner does not treat any character specially within text.
  Later stages can convert "escape sequences" as necessary, for
  example to accept C-style escaping (e.g. convert \n to a newline) or
  HTML-style escaping (e.g. convert &eacute; to é). In the XL
  implementation, this kind of conversion is outside the scope of the
  scanner.

  BLOBS:

  Blobs (binary large / lumped objects) represent arbitrary binary
  data. They begin with a '$' sign, followed by a sequence of
  hexadecimal digits. Blobs can be encoded in base 2, 4, 8, 16 and 64
  using the standard base notation (i.e. $16#FFFF00 or
  $64#TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpcyByZWFzb24sIGJ1dCBieS
  B0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhbmltYWxzLCB3aGljaCBpcyBhIGx1
  c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZXZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aG
  UgY29udGludWVkIGFuZCBpbmRlZmF0aWdhYmxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBl
  eGNlZWRzIHRoZSBzaG9ydCB2ZWhlbWVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=).

  Blobs can span multiple lines, and spaces are ignored. A single
  underscore can also be used for grouping like for normal numbers.


  BLANKS:

  In XL, indentation is significant, and represented internally by two
  special forms of parentheses, denoted as 'indent' and 'unindent'.
  Indentation can use space or tabs, but not both in the same source file.


  COMMENTS:

  The scanner doesn't decide what is a comment. This decision is taken by
  the caller (normally the parser). The "Comment" function can be called,
  and skips until an 'end of comment' token is found.
*/

#include "tree.h"
#include "text.h"
#include "number.h"
#include "position.h"
#include "syntax.h"

#ifdef SCANNER_C
#define inline extern inline
#endif

typedef enum token
// ----------------------------------------------------------------------------
//   Possible token types
// ----------------------------------------------------------------------------
{
    tokNONE = 0,

    // Normal conditions
    tokEOF,                     // End of file marker
    tokINTEGER,                 // Integer number
    tokREAL,                    // Real number,
    tokTEXT,                    // Double-quoted text
    tokCHARACTER,               // Single-quoted text
    tokLONGTEXT,                // Delimited text, e.g. << Hello >>
    tokNAME,                    // Alphanumeric name
    tokSYMBOL,                  // Punctuation symbol
    tokBLOB,                    // Binary object
    tokNEWLINE,                 // End of line
    tokOPEN,                    // Opening parenthese
    tokCLOSE,                   // Closing parenthese
    tokINDENT,                  // Indentation
    tokUNINDENT,                // Unindentation (one per indentation)

    // Error conditions
    tokERROR                    // Some error happened (normally hard to reach)
} token_t;


typedef union scanned
// ----------------------------------------------------------------------------
//    Possible outputs for the scanner
// ----------------------------------------------------------------------------
{
    tree_p          tree;
    text_p          text;
    name_p          name;
    character_p     character;
    natural_p       natural;
    based_natural_p based;
    real_p          real;
    blob_p          blob;
} scanned_t, *scanned_p;


blob_type(unsigned, indents);


typedef struct scanner
// ----------------------------------------------------------------------------
//    Internal representation of the XL scanner state
// ----------------------------------------------------------------------------
{
    positions_p positions;              // Description of file positions
    syntax_p    syntax;                 // Source code syntax
    tree_io_fn  reader;                 // Reading function
    void *      stream;                 // Stream we read from
    text_p      source;                 // Source form of the parsed token
    scanned_t   scanned;                // Scanned result
    indents_p   indents;                // Stack of indents
    text_p      block_close;            // Matching block close
    unsigned    indent;                 // Current level of indentation
    unsigned    column;                 // Current column during indentation
    char        pending_char[2];        // Read-ahead pending chars
    char        indent_char;            // To detect if mixing space/tabs
    bool        checking_indent  : 1;   // At beginning of line
    bool        setting_indent   : 1;   // Parenthesis sets indent
    bool        had_space_before : 1;   // Had space before token
    bool        had_space_after  : 1;   // Had space after token
} scanner_t, *scanner_p;


extern scanner_p scanner_new(positions_p positions, syntax_p syntax);
extern void      scanner_delete(scanner_p scan);
extern FILE *    scanner_open(scanner_p scan, const char *file);
extern void      scanner_close(scanner_p scan, FILE *f);
extern void      scanner_open_stream(scanner_p scan, const char *name,
                                     tree_io_fn reader, void *stream);
extern void      scanner_close_stream(scanner_p scan, void *stream);

extern token_t   scanner_read(scanner_p scan);
extern text_p    scanner_skip(scanner_p scan, name_p closing);

#undef inline

#endif // SCANNER_H
