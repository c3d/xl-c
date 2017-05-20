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
    syntax_r result = (syntax_r) tree_malloc(sizeof(syntax_t));

    array_set(&result->infixes, square_array_new(0, 0, NULL));
    array_set(&result->prefixes, square_array_new(0, 0, NULL));
    array_set(&result->postfixes, square_array_new(0, 0, NULL));

    array_set(&result->comments, square_array_new(0, 0, NULL));
    array_set(&result->texts, square_array_new(0, 0, NULL));
    array_set(&result->blocks, square_array_new(0, 0, NULL));

    array_set(&result->syntaxes, square_array_new(0, 0, NULL));

    if (file)
    {
        text_set(&result->filename, text_cnew(0, file));
        syntax_read_file(result, file);
    }

    return result;
}


tree_p syntax_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   Delete the given syntax configuration
// ----------------------------------------------------------------------------
{
    switch (cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "syntax";

    case TREE_SIZE:
        // Return the size of the tree in bytes (may be dynamic for subtypes)
        return (tree_p) sizeof(syntax_t);

    case TREE_ARITY:
        // There are ten children in a syntax
        return (tree_p) 10;

    case TREE_CHILDREN:
        // Return the pointer to children for that tree type
        return (tree_p) (tree + 1);

    default:
        return tree_handler(cmd, tree, va);
    }
}



// ============================================================================
//
//   Scanning a syntax file
//
// ============================================================================

static inline bool eq(text_p text, const char *str)
// ----------------------------------------------------------------------------
//    Compare the name value with a C string
// ----------------------------------------------------------------------------
{
    return text_eq(text, str);
}


static inline void set(text_p *text, const char *str)
// ----------------------------------------------------------------------------
//   Replace the text
// ----------------------------------------------------------------------------
{
    text_set(text, text_cnew(0, str));
}


static inline void set_priority(array_p *array, int priority, text_p text)
// ----------------------------------------------------------------------------
//   Define a priority in a priority array
// ----------------------------------------------------------------------------
{
    natural_p prio = natural_new(0, priority);
    array_push(array, (tree_r) text);
    array_push(array, (tree_r) prio);
}


static inline void set_delimiter(array_p *array, text_p open, text_p close)
// ----------------------------------------------------------------------------
//   Record a set of delimiters
// ----------------------------------------------------------------------------
{
    array_push(array, (tree_r) open);
    array_push(array, (tree_r) close);
}


static inline void set_syntax(array_p *array,
                              text_p open, text_p close, syntax_p syntax)
// ----------------------------------------------------------------------------
//   Record a set of delimiters
// ----------------------------------------------------------------------------
{
    array_push(array, (tree_r) open);
    array_push(array, (tree_r) close);
    array_push(array, (tree_r) syntax);
}


static int compare(const void *d1, const void *d2)
// ----------------------------------------------------------------------------
//   Comparison routine to sort priority arrays
// ----------------------------------------------------------------------------
{
    text_p *tp1 = (text_p *) d1;
    text_p *tp2 = (text_p *) d2;
    return text_compare(*tp1, *tp2);
}


static inline void sort(array_p array, size_t count)
// ----------------------------------------------------------------------------
//   Sort priority array
// ----------------------------------------------------------------------------
{
    size_t elem_size = count * sizeof(tree_p);
    qsort(array_data(array), array_length(array), elem_size, compare);
}


void syntax_read_file(syntax_r syntax, const char *filename)
// ----------------------------------------------------------------------------
//   Read a whole syntax file
// ----------------------------------------------------------------------------
{
    positions_p  positions = positions_new();
    scanner_p    scanner   = scanner_new(positions, NULL);
    FILE        *file      = scanner_open(scanner, filename);

    syntax_read(syntax, scanner);

    scanner_close(scanner, file);
    scanner_delete(scanner);
    positions_delete(positions);
}


void syntax_read(syntax_r syntax, scanner_p scanner)
// ----------------------------------------------------------------------------
//   Read syntax from the given scanner (either a whole file or a source code)
// ----------------------------------------------------------------------------
{
    typedef enum state
    {
        UNKNOWN, PREFIX, INFIX, POSTFIX,
        COMMENT, COMMENT_END,
        TEXT, TEXT_END,
        BLOCK, BLOCK_END,
        SYNTAX_NAME, SYNTAX, SYNTAX_END
    } state_t;

    state_t  state    = UNKNOWN;
    int      priority = 0;
    text_p   entry    = NULL;
    syntax_p child    = NULL;
    token_t  token    = tokNONE;
    unsigned indent   = 0;
    bool     done     = false;

    while (!done)
    {
        text_p known_token = NULL;
        text_p text = NULL;

        token = scanner_read(scanner);

        switch(token)
        {
        case tokINTEGER:
            priority = natural_value(scanner->scanned.natural);
            break;
        case tokTEXT:
        case tokCHARACTER:
        case tokSYMBOL:
            known_token = text;
            /* Fall through */

        case tokNAME:
            text = scanner->scanned.text;

            if (eq(text, "NEWLINE"))
                set(&text, "\n");
            else if (eq(text, "INDENT"))
                set(&text, SYNTAX_INDENT);
            else if (eq(text, "UNINDENT"))
                set(&text, SYNTAX_UNINDENT);

            else if (eq(text, "INFIX"))
                state = INFIX;
            else if (eq(text, "PREFIX"))
                state = PREFIX;
            else if (eq(text, "POSTFIX"))
                state = POSTFIX;
            else if (eq(text, "BLOCK"))
                state = BLOCK;
            else if (eq(text, "COMMENT"))
                state = COMMENT;
            else if (eq(text, "TEXT"))
                state = TEXT;
            else if (eq(text, "SYNTAX"))
                state = SYNTAX_NAME;

            else if (eq(text, "STATEMENT"))
                syntax->statement_priority = priority;
            else if (eq(text, "FUNCTION"))
                syntax->function_priority = priority;
            else if (eq(text, "DEFAULT"))
                syntax->default_priority = priority;

            else switch(state)
            {
            case UNKNOWN:
                break;
            case PREFIX:
                set_priority(&syntax->prefixes, priority, text);
                break;
            case POSTFIX:
                set_priority(&syntax->postfixes, priority, text);
                break;
            case INFIX:
                set_priority(&syntax->infixes, priority, text);
                break;
            case COMMENT:
                text_set(&entry, (text_r) text);
                state = COMMENT_END;
                break;
            case COMMENT_END:
                set_delimiter(&syntax->comments, entry, text);
                state = COMMENT;
                break;
            case TEXT:
                text_set(&entry, (text_r) text);
                state = TEXT_END;
                break;
            case TEXT_END:
                set_delimiter(&syntax->texts, entry, text);
                state = TEXT;
                break;
            case BLOCK:
                text_set(&entry, (text_r) text);
                state = BLOCK_END;
                set_priority(&syntax->infixes, priority, text);
                break;
            case BLOCK_END:
                set_delimiter(&syntax->blocks, entry, text);
                set_priority(&syntax->infixes, priority, text);
                state = BLOCK;
                break;
            case SYNTAX_NAME:
                // Nul-terminate file name
                text_append_data(&text, 1, NULL);
                child = syntax_new(text_data(text));
                state = SYNTAX;
                break;
            case SYNTAX:
                text_set(&entry, (text_r) text);
                state = SYNTAX_END;
                break;
            case SYNTAX_END:
                set_syntax(&syntax->syntaxes, entry, text, child);
                state = SYNTAX;
                break;
            }
            break;

        case tokEOF:
            done = true;
            break;

        case tokINDENT:
            indent++;
            break;
        case tokUNINDENT:
            done = (--indent == 0);
            break;

        default:
            // Any other stuff (indents, etc) is skipped
            break;
        } // switch

        // If we read an operator name, insert it in list of known operators
        if (known_token)
            array_push(&syntax->known, (tree_r) known_token);

        text_dispose(&text);
    } // while

    // Sort the various arrays
    sort(syntax->known, 1);

    sort(syntax->infixes, 2);
    sort(syntax->prefixes, 2);
    sort(syntax->postfixes, 2);

    sort(syntax->comments, 2);
    sort(syntax->texts, 2);
    sort(syntax->blocks, 2);

    sort(syntax->syntaxes, 3);

    text_dispose(&entry);
}



// ============================================================================
//
//   Checking syntax elements
//
// ============================================================================

static int search_internal(array_p array, text_p key, size_t skip,
                           unsigned first, unsigned last)
// ----------------------------------------------------------------------------
//   Binary search on sorted entries in array
// ----------------------------------------------------------------------------
{
    tree_p *data = array_data(array);
    int     mid  = (first + last) / 2;

    while (first < last)
    {
        int cmp  = compare(data + mid * skip, &key);
        if (cmp == 0)
            return mid;
        if(cmp > 0)
            first = mid;
        else
            last = mid;
        mid  = (first + last) / 2;
    }

    // Not found - Return closest location
    return ~mid;
}

static inline int search(array_p array, text_p key, size_t skip)
// ----------------------------------------------------------------------------
//   Binary search on array
// ----------------------------------------------------------------------------
{
    return search_internal(array, key, skip, 0, array_length(array));
}


int syntax_infix_priority(syntax_p s, text_p name)
// ----------------------------------------------------------------------------
//    Return the priority for the given infix, or default_priority
// ----------------------------------------------------------------------------
{
    int index = search(s->infixes, name, 2);
    if (index >= 0)
    {
        natural_p n = (natural_p) array_child(s->infixes, index+1);
        return natural_value(n);
    }
    return s->default_priority;

}


int syntax_prefix_priority(syntax_p s, text_p name)
// ----------------------------------------------------------------------------
//    Return the priority for the given infix, or default_priority
// ----------------------------------------------------------------------------
{
    int index = search(s->prefixes, name, 2);
    if (index >= 0)
    {
        natural_p n = (natural_p) array_child(s->prefixes, index+1);
        return natural_value(n);
    }
    return s->default_priority;

}


int syntax_postfix_priority(syntax_p s, text_p name)
// ----------------------------------------------------------------------------
//    Return the priority for the given infix, or default_priority
// ----------------------------------------------------------------------------
{
    int index = search(s->postfixes, name, 2);
    if (index >= 0)
    {
        natural_p n = (natural_p) array_child(s->postfixes, index+1);
        return natural_value(n);
    }
    return s->default_priority;

}


bool syntax_is_operator(syntax_p s, text_p name)
// ----------------------------------------------------------------------------
//   Check if the given name is a known operator
// ----------------------------------------------------------------------------
{
    int where = search(s->known, name, 1);

    // Check if we had an exact match
    if (where >= 0)
        return true;

    // Check if we had a partial match
    // If there were an entry like AACD, ABCD, ABCE and we searched ABC,
    // The mid point where we stopped can be AACD or ABCD
    size_t length = array_length(s->known);
    size_t closest = ~where;
    for (size_t index = closest; index <= closest+1; index++)
    {
        if (index < length)
        {
            text_p key = (text_p) array_child(s->known, index);
            if (text_length(name) < text_length(key) &&
                memcmp(text_data(name), text_data(key), text_length(name)) == 0)
                return true;
        }
    }

    return false;
}


bool syntax_is_block(syntax_p s, text_p name, text_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->blocks, name, 2);
    if (index >= 0)
    {
        text_set(closing, (text_r) array_child(s->blocks, index+1));
        return true;
    }
    return false;
}


bool syntax_is_text(syntax_p s, text_p name, text_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->texts, name, 2);
    if (index >= 0)
    {
        text_set(closing, (text_r) array_child(s->texts, index+1));
        return true;
    }
    return false;
}


bool syntax_is_comment(syntax_p s, text_p name, text_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->comments, name, 2);
    if (index >= 0)
    {
        text_set(closing, (text_r) array_child(s->comments, index+1));
        return true;
    }
    return false;
}
