// ****************************************************************************
//  syntax.c                                        XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Internal description of the XL syntax file
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

#define SYNTAX_C
#include "syntax.h"
#include "scanner.h"

#include <stdlib.h>
#include <string.h>


syntax_p syntax_new(const char *file)
// ----------------------------------------------------------------------------
//   Create a new syntax configuration, normally from a syntax file
// ----------------------------------------------------------------------------
{
    // Zero-initialize the memory
    syntax_p result = calloc(1, sizeof(syntax_t));

    array_set(&result->infix, square_array_new(0, 0, NULL));
    array_set(&result->prefix, square_array_new(0, 0, NULL));
    array_set(&result->postfix, square_array_new(0, 0, NULL));

    array_set(&result->comment, square_array_new(0, 0, NULL));
    array_set(&result->text, square_array_new(0, 0, NULL));
    array_set(&result->block, square_array_new(0, 0, NULL));

    array_set(&result->syntax, square_array_new(0, 0, NULL));
    syntaxes_set(&result->syntaxes, syntaxes_new(0, 0, NULL));

    if (file)
    {
        text_set(&result->filename, text_cnew(0, file));


    }


    return result;
}


void syntax_delete(syntax_p s)
// ----------------------------------------------------------------------------
//   Delete the given syntax configuration
// ----------------------------------------------------------------------------
{
    text_dispose(&s->filename);

    array_dispose(&s->infix);
    array_dispose(&s->prefix);
    array_dispose(&s->postfix);

    array_dispose(&s->comment);
    array_dispose(&s->text);
    array_dispose(&s->block);

    array_dispose(&s->syntax);
    syntaxes_dispose(&s->syntaxes);

    free(s);
}



// ============================================================================
//
//   Checking syntax elements
//
// ============================================================================

bool syntax_is_operator(syntax_p s, text_p name)
// ----------------------------------------------------------------------------
//   Check if the given name is a known operator
// ----------------------------------------------------------------------------
{
    return false;
}


bool syntax_is_block_open(syntax_p s, name_p name)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    return false;
}


bool syntax_is_block_close(syntax_p s, name_p name)
// ----------------------------------------------------------------------------
//    Check if the given name closes a block
// ----------------------------------------------------------------------------
{
    return false;
}


bool syntax_is_block_open_character(syntax_p s, char name)
// ----------------------------------------------------------------------------
//   Check if the given character opens a block, e.g. ( or [
// ----------------------------------------------------------------------------
{
    return false;
}


bool syntax_is_block_close_character(syntax_p s, char name)
// ----------------------------------------------------------------------------
//   Check if the given character closes a block, e.g. ) or ]
// ----------------------------------------------------------------------------
{
    return false;
}
