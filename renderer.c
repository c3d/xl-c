// ****************************************************************************
//  renderer.c                                      XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Rendering parse trees in a human-friendly form
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

#include "renderer.h"

#include "array.h"
#include "block.h"
#include "error.h"
#include "scanner.h"
#include "text.h"

#include <ctype.h>



// ============================================================================
//
//   Structure holding the internal state of a renderer
//
// ============================================================================

typedef struct renderer
// ----------------------------------------------------------------------------
//    Structure holding rendering information
// ----------------------------------------------------------------------------
{
    // Configuration
    tree_io_fn          output;                 // Output function
    void *              stream;                 // Output stream
    syntax_p            syntax;                 // Syntax (priorities, etc)
    array_p             formats;                // Format for keywords
    text_p              cr;                     // A text containing '\n'
    text_p              space;                  // A text containing ' '
    text_p              indent;                 // A text containing 'indent'
    text_p              begin;                  // A text containing 'begin'
    text_p              end;                    // A text containing 'end'

    // Dynamic state
    tree_p              self;                   // 'self' keyword
    int                 priority;               // Current priority
    unsigned            indents;                // Current indentation level
    char                quote;                  // Quote when escaping
    bool                had_space       : 1;    // Emitted space just before
    bool                had_newline     : 1;    // Emitted newline just before
    bool                had_punctuation : 1;    // Emitted punctuation
    bool                need_separator  : 1;    // Separator needed after
    bool                need_newline    : 1;    // Newline needed after
} renderer_t, *renderer_p;



// ============================================================================
//
//   Constructor and destructor
//
// ============================================================================

renderer_p renderer_new(const char *style)
// ----------------------------------------------------------------------------
//   Create a default-initialized renderer
// ----------------------------------------------------------------------------
{
    renderer_p result = malloc(sizeof(renderer_t));
    memset(result, 0, sizeof(renderer_t));
    text_set(&result->cr, text_cnew(0, "\n"));
    text_set(&result->space, text_cnew(0, " "));
    text_set(&result->indent, text_cnew(0, "indent"));
    text_set(&result->begin, text_cnew(0, "begin"));
    text_set(&result->end, text_cnew(0, "end"));
    if (style)
        renderer_style(result, style);
    return result;
}


void renderer_delete(renderer_p renderer)
// ----------------------------------------------------------------------------
//   Delete a renderer
// ----------------------------------------------------------------------------
{
    syntax_dispose(&renderer->syntax);
    array_dispose(&renderer->formats);
    tree_dispose(&renderer->self);
    text_dispose(&renderer->cr);
    text_dispose(&renderer->indent);
    text_dispose(&renderer->space);
    text_dispose(&renderer->begin);
    text_dispose(&renderer->end);
    free(renderer);
}



// ============================================================================
//
//    Renderer attributes
//
// ============================================================================

syntax_p renderer_syntax(renderer_p renderer, syntax_p syntax)
// ----------------------------------------------------------------------------
//   Set the syntax for the renderer, return previous one
// ----------------------------------------------------------------------------
{
    syntax_p result = renderer->syntax;
    syntax_set(&renderer->syntax, syntax);
    return result;
}


tree_io_fn renderer_output_function(renderer_p renderer, tree_io_fn function)
// ----------------------------------------------------------------------------
//   Set the output function for the renderer, return previous one
// ----------------------------------------------------------------------------
{
    tree_io_fn result = renderer->output;
    renderer->output = function;
    return result;
}


void *renderer_output_stream(renderer_p renderer, void *stream)
// ----------------------------------------------------------------------------
//    Set the output stream for the renderer, return previous one
// ----------------------------------------------------------------------------
{
    void *result = renderer->stream;
    renderer->stream = stream;
    return result;
}


static inline bool eq(text_p text, const char *str)
// ----------------------------------------------------------------------------
//    Compare the name value with a C string
// ----------------------------------------------------------------------------
{
    return text_eq(text, str);
}


void renderer_style(renderer_p renderer, const char  *style)
// ----------------------------------------------------------------------------
//    Load a style from the given style file
// ----------------------------------------------------------------------------
{
    positions_p  positions       = error_positions();
    scanner_p    scanner         = scanner_new(positions, NULL);
    FILE        *file            = scanner_open(scanner, style);
    text_p       entry           = NULL;
    array_p      format          = NULL;
    array_p      formats         = array_use(array_new(0, 0, NULL));
    token_t      token           = tokNONE;
    name_p       end_c_comment   = name_cnew(0, "*/");
    name_p       end_cpp_comment = name_cnew(0, "\n");
    unsigned     indent          = 0;
    bool         done            = false;
    srcpos_t     equal_pos       = 0;

    while (!done)
    {
        text_p source     = NULL;
        bool   enter_last = false;

        token = scanner_read(scanner);

        switch(token)
        {
        case tokTEXT:
        case tokCHARACTER:
        case tokSYMBOL:
        case tokNAME:
            text_set(&source, scanner->source);

            if (eq(source, "="))
            {
                srcpos_t pos = position(positions);
                if (equal_pos)
                {
                    error(pos, "We already had an equal sign");
                    error(equal_pos, "Position of previous equal sign");
                }
                equal_pos = pos;
                if (!entry)
                {
                    error(pos, "No text or symbol precedes equal sign");
                }
                array_set(&format, array_new(pos, 0, NULL));
            }
            else if (eq(source, "/*"))
            {
                text_p comment = scanner_skip(scanner, end_c_comment);
                text_dispose(&comment);
            }
            else if (eq(source, "//"))
            {
                text_p comment = scanner_skip(scanner, end_cpp_comment);
                text_dispose(&comment);
            }
            else if (format)
            {
                array_push(&format, (tree_p) source);
            }
            else if (!entry)
            {
                if (token == tokTEXT)
                    text_set(&entry, scanner->scanned.text);
                else
                    text_set(&entry, source);
            }
            else
            {
                srcpos_t pos = position(positions);
                error(pos, "Unexpected name %t following %t", source, entry);
            }
            break;

        case tokEOF:
            done = true;
            enter_last = true;
            break;

        case tokNEWLINE:
            equal_pos = 0;
            enter_last = true;
            break;

        case tokINDENT:
            indent++;
            break;
        case tokUNINDENT:
            if (--indent == 0)
            {
                equal_pos = 0;
                enter_last = true;
            }
            break;

        default:
            // Any other stuff (indents, etc) is skipped
            error(position(positions), "Unexpected token %t", scanner->source);
            break;
        } // switch

        // If we need to enter the last format
        if (enter_last && entry && format)
        {
            equal_pos = 0;
            array_push(&formats, (tree_p) entry);
            array_push(&formats, (tree_p) format);
            text_dispose(&entry);
            array_dispose(&format);
        }
        text_dispose(&source);
    } // while

    // Sort the formats for faster search
    array_sort(formats, (compare_fn) text_compare, 2);
    array_set(&renderer->formats, formats);

    // Cleanup
    text_dispose(&entry);
    name_dispose(&end_cpp_comment);
    name_dispose(&end_c_comment);
    array_dispose(&format);
    array_dispose(&formats);

    scanner_close(scanner, file);
    scanner_delete(scanner);
}


void renderer_reset(renderer_p renderer)
// ----------------------------------------------------------------------------
//   Reset renderer to initial rendering state
// ----------------------------------------------------------------------------
{
    tree_set(&renderer->self, NULL);
    renderer->priority = 0;
    renderer->indents = 0;
    renderer->quote = 0;
    renderer->had_space = true;
    renderer->had_newline = true;
    renderer->had_punctuation = false;
    renderer->need_separator = false;
    renderer->need_newline = false;
}



// ============================================================================
//
//   Rendering proper
//
// ============================================================================

static bool   render_format(renderer_p renderer, text_p format);
static void   render_separators(renderer_p renderer, char next);
static void   render_indents(renderer_p renderer);
static bool   render_child(renderer_p renderer, unsigned index);


void render_file(renderer_p r, tree_p tree)
// ----------------------------------------------------------------------------
//    Render a tree in the given renderer, considerered as a whole file
// ----------------------------------------------------------------------------
{
    renderer_reset(r);
    render_format(r, r->begin);
    render(r, tree);
    render_format(r, r->end);
}


void render(renderer_p r, tree_p tree)
// ----------------------------------------------------------------------------
//    Render the tree using the tree handler
// ----------------------------------------------------------------------------
{
    const char *type = tree_typename(tree);
    text_p format = text_cnew(tree_position(tree), type);
    tree_p save_self = tree_use(r->self);
    tree_set(&r->self, tree);
    if (!render_format(r, format))
        tree_io(TREE_RENDER, tree, r);
    text_dispose(&format);
    tree_set(&r->self, save_self);
    tree_dispose(&save_self);
}


static bool render_child(renderer_p r, unsigned child)
// ----------------------------------------------------------------------------
//   Render the nth child
// ----------------------------------------------------------------------------
{
    tree_p self = r->self;
    if (self)
    {
        size_t arity = tree_arity(self);
        if (child < arity)
        {
            render(r, tree_child(self, child));
            return true;
        }
    }
    return false;
}


static bool render_format(renderer_p r, text_p format)
// ----------------------------------------------------------------------------
//   Render a given format if found
// ----------------------------------------------------------------------------
{
    // Find if format has text form, e.g "ABC" or 'ABC'
    size_t length = text_length(format);
    if (length >= 2)
    {
        char *data = text_data(format);
        if ((data[0] == '"' || data[0] == '\'') && data[length-1] == data[0])
        {
            // Render text as is, no quote
            render_text(r, length - 2, data + 1);
            return true;
        }
    }

#define KEYWORD(x)   if (eq(format, x))

    KEYWORD("self")
    {
        tree_io(TREE_RENDER, r->self, r);
        return true;
    }
    KEYWORD("indent")
    {
        r->indents += 1;
        return true;
    }
    KEYWORD("unindent")
    {
        r->indents -= 1;
        return true;
    }
    KEYWORD("indents")
    {
        render_indents(r);
        return true;
    }
    KEYWORD("separator")
    {
        r->need_separator = true;
        return true;
    }
    KEYWORD("cr")
    {
        r->need_newline = true;
        return true;
    }
    KEYWORD("newline")
    {
        r->need_newline = true;
        return true;
    }
    KEYWORD("child")
    {
        if (render_child(r, 0))
            return true;
    }
    KEYWORD("left")
    {
        if (render_child(r, 0))
            return true;
    }
    KEYWORD("right")
    {
        if (render_child(r, 1))
            return true;
    }
    KEYWORD("opcode")
    {
        if (render_child(r, 2))
            return true;
    }
    KEYWORD("block_opening")
    {
        block_p block = tree_cast(block, r->self);
        if (block)
            render(r, (tree_p) block->opening);
        return true;
    }
    KEYWORD("block_closing")
    {
        block_p block = tree_cast(block, r->self);
        if (block)
            render(r, (tree_p) block->closing);
        return true;
    }
    KEYWORD("block_separator")
    {
        block_p block = tree_cast(block, r->self);
        if (block)
            render(r, (tree_p) block->separator);
        return true;
    }
    KEYWORD("space")
    {
        if (!r->had_space)
            render_text(r, 1, " ");
        return true;
    }

    // Find if format was declared in style sheet
    if (r->formats)
    {
        int index = array_search(r->formats,
                                 (tree_p)format,
                                 (compare_fn) text_compare,
                                 2);
        if (index >= 0)
        {
            // It was found: render all elements in array in turn
            array_p seq  = (array_p) array_child(r->formats, 2*index+1);
            size_t  len  = array_length(seq);
            text_p *data = (text_p *) array_data(seq);
            bool    err  = false;
            for (size_t i = 0; i < len; i++)
            {
                if (!render_format(r, *data++))
                {
                    if (err == false)
                    {
                        err = true;
                        error(text_position(format),
                              "While rendering %t", format);
                        error(text_position(data[-1]),
                              "Invalid format directive %t", data[-1]);
                    }
                }
            }
            return true;
        }
    }

    // Nothing found
    return false;
}


void render_text(renderer_p r, size_t length, const char *data)
// ----------------------------------------------------------------------------
//   Send the given text to the output
// ----------------------------------------------------------------------------
{
    srcpos_t pos = tree_position(r->self);
    for (size_t i = 0; i < length; i++)
    {
        char c = data[i];
        if (r->need_newline || r->need_separator || r->had_newline)
        {
            render_separators(r, c);
            if (r->had_newline && i == 0 && c == '\n')
                continue;
        }

        if (c == '\n')
        {
            r->need_newline = true;
            r->need_separator = false;
        }
        else
        {
            bool quoted = c == r->quote;
            text_p format = text_new(pos, 1, &c);
            if (render_format(r, format))
            {
                if (quoted)
                    render_format(r, format);
            }
            else
            {
                r->output(r->stream, 1, &c);
                if (quoted)
                    r->output(r->stream, 1, &c); // As in """Hello"""
            }
            text_dispose(&format);
        }
        r->had_space = isspace(c);
        r->had_punctuation = ispunct(c);
    }
}


void render_open_quote(renderer_p r, char quote)
// ----------------------------------------------------------------------------
//   Write out the open quote
// ----------------------------------------------------------------------------
{
    if (r->quote)
        error(tree_position(r->self),
              "Two quotes for %t, had %c now %c", r->self, r->quote, quote);
    render_text(r, 1, &quote);
    r->quote = quote;
}


void render_close_quote(renderer_p r, char quote)
// ----------------------------------------------------------------------------
//    Close quote
// ----------------------------------------------------------------------------
{
    if (!r->quote)
        error(tree_position(r->self),
              "No quote for %t, closing %c", r->self, quote);
    else if (r->quote != quote)
        error(tree_position(r->self), "Mismatched quote for %t, had %c, now %c",
              r->self,r->quote,quote);
    r->quote = 0;
    render_text(r, 1, &quote);
}


void render_separators(renderer_p r, char next)
// ----------------------------------------------------------------------------
//   Render space and newline separators as required before 'next'
// ----------------------------------------------------------------------------
{
    if (r->need_newline)
    {
        r->had_newline = true;
        r->need_newline = false;
        if (!render_format(r, r->cr))
            r->output(r->stream, 1, "\n");
    }

    if (next != '\n')
    {
        if (r->had_newline && next != 0)
        {
            r->had_newline = false;
            r->need_separator = false;
            render_indents(r);
        }

        if (r->need_separator)
        {
            r->need_separator = false;
            if (!r->had_space && !isspace(next))
            {
                if (r->had_punctuation == ispunct(next))
                {
                    if (!render_format(r, r->space))
                        r->output(r->stream, 1, " ");
                }
            }
        }
    }
}


void render_indents(renderer_p r)
// ----------------------------------------------------------------------------
//   Render indentation level
// ----------------------------------------------------------------------------
{
    for (unsigned i = 0; i < r->indents; i++)
        if (!render_format(r, r->indent))
            render_text(r, 4, "    ");
}
