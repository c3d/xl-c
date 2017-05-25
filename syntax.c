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
    syntax_p result = (syntax_p) tree_malloc(sizeof(syntax_t));
    result->tree.handler = syntax_handler;

    result->known = array_use(array_new(0, 0, NULL));

    result->infixes = array_use(array_new(0, 0, NULL));
    result->prefixes = array_use(array_new(0, 0, NULL));
    result->postfixes = array_use(array_new(0, 0, NULL));

    result->comments = array_use(array_new(0, 0, NULL));
    result->texts = array_use(array_new(0, 0, NULL));
    result->blocks = array_use(array_new(0, 0, NULL));

    result->syntaxes = array_use(array_new(0, 0, NULL));

    if (file)
    {
        result->filename = text_use(text_cnew(0, file));
        syntax_read_file(result, file);
    }

    return result;
}


tree_p syntax_handler(tree_cmd_t cmd, tree_p tree, va_list va)
// ----------------------------------------------------------------------------
//   Delete the given syntax configuration
// ----------------------------------------------------------------------------
{
    syntax_p   s = (syntax_p) tree;
    tree_io_fn io;
    void *     stream;

    switch (cmd)
    {
    case TREE_TYPENAME:
        // Return a default tree type name
        return (tree_p) "syntax";

    case TREE_SIZE:
        // Return the size of the tree in bytes (may be dynamic for subtypes)
        return (tree_p) sizeof(syntax_t);

    case TREE_ARITY:
        // All tree-type fields in the syntax
        return (tree_p) 9;

    case TREE_CAST:
        // Check if we cast to blob type, if so, success
        if (tree_cast_handler(va) == syntax_handler)
            return tree;
        break;                      // Pass on to base class handler

    case TREE_CHILDREN:
        // Return the pointer to children for that tree type
        return (tree_p) (tree + 1);

    case TREE_RENDER:
        io = va_arg(va, tree_io_fn);
        stream = va_arg(va, void *);

#define ioputs(child)                                   \
        io(stream, sizeof(#child "=")-1, #child "=");   \
        tree_render((tree_p) s->child, io, stream);     \
        io(stream, 1, "\n");

        ioputs(filename);
        ioputs(known);
        ioputs(infixes);
        ioputs(prefixes);
        ioputs(postfixes);
        ioputs(comments);
        ioputs(texts);
        ioputs(blocks);
        ioputs(syntaxes);

        break;

    default:
        break;
    }
    return tree_handler(cmd, tree, va);
}



// ============================================================================
//
//   Scanning a syntax file
//
// ============================================================================

static inline bool eq(name_p text, const char *str)
// ----------------------------------------------------------------------------
//    Compare the name value with a C string
// ----------------------------------------------------------------------------
{
    return name_eq(text, str);
}


static inline void set(name_p *text, const char *str)
// ----------------------------------------------------------------------------
//   Replace the text
// ----------------------------------------------------------------------------
{
    name_set(text, name_cnew(0, str));
}


static inline void set_priority(array_p *array, int priority, name_p name)
// ----------------------------------------------------------------------------
//   Define a priority in a priority array
// ----------------------------------------------------------------------------
{
    natural_p prio = natural_new(0, priority);
    array_push(array, (tree_p) name);
    array_push(array, (tree_p) prio);
}


static inline void set_delimiter(array_p *array, name_p open, name_p close)
// ----------------------------------------------------------------------------
//   Record a set of delimiters
// ----------------------------------------------------------------------------
{
    array_push(array, (tree_p) open);
    array_push(array, (tree_p) close);
}


static inline void set_syntax(array_p *array,
                              name_p open, name_p close, syntax_p syntax)
// ----------------------------------------------------------------------------
//   Record a set of delimiters
// ----------------------------------------------------------------------------
{
    array_push(array, (tree_p) open);
    array_push(array, (tree_p) close);
    array_push(array, (tree_p) syntax);
}


static int compare(const void *d1, const void *d2)
// ----------------------------------------------------------------------------
//   Comparison routine to sort priority arrays
// ----------------------------------------------------------------------------
{
    name_p *tp1 = (name_p *) d1;
    name_p *tp2 = (name_p *) d2;
    return name_compare(*tp1, *tp2);
}


static inline void sort(array_p array, size_t count)
// ----------------------------------------------------------------------------
//   Sort priority array
// ----------------------------------------------------------------------------
{
    size_t elem_size = count * sizeof(tree_p);
    qsort(array_data(array), array_length(array)/count, elem_size, compare);
}


void syntax_read_file(syntax_p syntax, const char *filename)
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


void syntax_read(syntax_p syntax, scanner_p scanner)
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
    name_p   entry    = NULL;
    syntax_p child    = NULL;
    token_t  token    = tokNONE;
    unsigned indent   = 0;
    bool     done     = false;

    while (!done)
    {
        name_p known_token = NULL;
        name_p name = NULL;

        token = scanner_read(scanner);

        switch(token)
        {
        case tokINTEGER:
            priority = natural_value(scanner->scanned.natural);
            break;
        case tokTEXT:
        case tokCHARACTER:
        case tokSYMBOL:
            name_set(&known_token, scanner->scanned.name);
            /* Fall through */

        case tokNAME:
            name_set(&name, scanner->scanned.name);

            if (eq(name, "NEWLINE"))
                set(&name, "\n");
            else if (eq(name, "INDENT"))
                set(&name, SYNTAX_INDENT);
            else if (eq(name, "UNINDENT"))
                set(&name, SYNTAX_UNINDENT);

            else if (eq(name, "INFIX"))
                state = INFIX;
            else if (eq(name, "PREFIX"))
                state = PREFIX;
            else if (eq(name, "POSTFIX"))
                state = POSTFIX;
            else if (eq(name, "BLOCK"))
                state = BLOCK;
            else if (eq(name, "COMMENT"))
                state = COMMENT;
            else if (eq(name, "TEXT"))
                state = TEXT;
            else if (eq(name, "SYNTAX"))
                state = SYNTAX_NAME;

            else if (eq(name, "STATEMENT"))
                syntax->statement_priority = priority;
            else if (eq(name, "FUNCTION"))
                syntax->function_priority = priority;
            else if (eq(name, "DEFAULT"))
                syntax->default_priority = priority;

            else switch(state)
            {
            case UNKNOWN:
                break;
            case PREFIX:
                set_priority(&syntax->prefixes, priority, name);
                break;
            case POSTFIX:
                set_priority(&syntax->postfixes, priority, name);
                break;
            case INFIX:
                set_priority(&syntax->infixes, priority, name);
                break;
            case COMMENT:
                name_set(&entry, (name_p) name);
                state = COMMENT_END;
                break;
            case COMMENT_END:
                set_delimiter(&syntax->comments, entry, name);
                state = COMMENT;
                break;
            case TEXT:
                name_set(&entry, (name_p) name);
                state = TEXT_END;
                break;
            case TEXT_END:
                set_delimiter(&syntax->texts, entry, name);
                state = TEXT;
                break;
            case BLOCK:
                name_set(&entry, (name_p) name);
                state = BLOCK_END;
                set_priority(&syntax->infixes, priority, name);
                break;
            case BLOCK_END:
                set_delimiter(&syntax->blocks, entry, name);
                set_priority(&syntax->infixes, priority, name);
                state = BLOCK;
                break;
            case SYNTAX_NAME:
                // Nul-terminate file name
                name_append_data(&name, 1, NULL);
                child = syntax_new(name_data(name));
                state = SYNTAX;
                break;
            case SYNTAX:
                name_set(&entry, (name_p) name);
                state = SYNTAX_END;
                break;
            case SYNTAX_END:
                set_syntax(&syntax->syntaxes, entry, name, child);
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
            array_push(&syntax->known, (tree_p) known_token);

        name_dispose(&name);
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

    name_dispose(&entry);
}



// ============================================================================
//
//   Checking syntax elements
//
// ============================================================================

static int search_internal(array_p array, name_p key, size_t skip,
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

static inline int search(array_p array, name_p key, size_t skip)
// ----------------------------------------------------------------------------
//   Binary search on array
// ----------------------------------------------------------------------------
{
    return search_internal(array, key, skip, 0, array_length(array));
}


int syntax_infix_priority(syntax_p s, name_p name)
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


int syntax_prefix_priority(syntax_p s, name_p name)
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


int syntax_postfix_priority(syntax_p s, name_p name)
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


bool syntax_is_operator(syntax_p s, name_p name)
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
            name_p key = (name_p) array_child(s->known, index);
            if (name_length(name) < name_length(key) &&
                memcmp(name_data(name), name_data(key), name_length(name)) == 0)
                return true;
        }
    }

    return false;
}


bool syntax_is_block(syntax_p s, name_p name, name_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->blocks, name, 2);
    if (index >= 0)
    {
        name_set(closing, (name_p) array_child(s->blocks, index+1));
        return true;
    }
    return false;
}


bool syntax_is_text(syntax_p s, name_p name, name_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->texts, name, 2);
    if (index >= 0)
    {
        name_set(closing, (name_p) array_child(s->texts, index+1));
        return true;
    }
    return false;
}


bool syntax_is_comment(syntax_p s, name_p name, name_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a block
// ----------------------------------------------------------------------------
{
    int index = search(s->comments, name, 2);
    if (index >= 0)
    {
        name_set(closing, name_ptr(array_child(s->comments, index+1)));
        return true;
    }
    return false;
}


syntax_p syntax_is_special(syntax_p s, name_p name, name_p *closing)
// ----------------------------------------------------------------------------
//    Check if the given name opens a child syntax
// ----------------------------------------------------------------------------
{
    int index = search(s->comments, name, 3);
    if (index >= 0)
    {
        name_set(closing, name_ptr(array_child(s->comments, index+1)));
        return syntax_ptr(array_child(s->comments, index+2));
    }
    return NULL;
}
