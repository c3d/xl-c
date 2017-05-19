// ****************************************************************************
//  scanner.c                                       XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//    Scanner for the XL programming language
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

#define SCANNER_C
#include "scanner.h"

#include "error.h"
#include "name.h"
#include "utf8.h"

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


// Workaround for now
#define syntax_is_block_open(...) false
#define syntax_is_block_close(...) false
#define syntax_is_block_open_character(...) false
#define syntax_is_block_close_character(...) false
#define syntax_known(...) false



// ============================================================================
//
//    Scanner management functions
//
// ============================================================================

scanner_p scanner_new(positions_p positions)
// ----------------------------------------------------------------------------
//    Create a new scanner
// ----------------------------------------------------------------------------
{
    scanner_p s = malloc(sizeof(scanner_t));
    s->positions = positions;
    s->reader = NULL;
    s->stream = NULL;
    s->source = NULL;
    s->scanned.text = NULL;
    s->indents = indents_new(position(positions), 0, NULL);
    s->indent = 0;
    s->column = 0;
    s->pending_char[0] = 0;
    s->pending_char[1] = 0;
    s->indent_char = 0;
    s->reading_syntax  = false;
    s->checking_indent = false;
    s->setting_indent = false;
    s->had_space_before = false;
    s->had_space_after = false;
    return false;
}


void scanner_delete(scanner_p s)
// ----------------------------------------------------------------------------
//    Delete the given scanner
// ----------------------------------------------------------------------------
{
    text_dispose(&s->source);
    text_dispose(&s->scanned.text);
    indents_dispose(&s->indents);
    free(s);
}


static unsigned scanner_file_read(void *stream, unsigned size, void *data)
// ----------------------------------------------------------------------------
//   Write data to the given FILE * stream
// ----------------------------------------------------------------------------
{
    FILE *input = (FILE *) stream;
    return fread(data, 1, size, input);
}


FILE *scanner_open(scanner_p s, const char *file)
// ----------------------------------------------------------------------------
//    Open the given file in the scanner
// ----------------------------------------------------------------------------
{
    FILE *f = fopen(file, "r");
    if (f)
        scanner_open_stream(s, file, scanner_file_read, f);
    return f;
}


void scanner_close(scanner_p s, FILE *f)
// ----------------------------------------------------------------------------
//    Close the given scanner file
// ----------------------------------------------------------------------------
{
    fclose(f);
    scanner_close_stream(s, f);
}


void scanner_open_stream(scanner_p s, const char *name,
                         tree_io_fn reader, void *stream)
// ----------------------------------------------------------------------------
//   Open a stream with the given I/O function
// ----------------------------------------------------------------------------
{
    assert(s->reader == NULL && "Cannot open a scanner that is already open");
    s->reader = reader;
    s->stream = stream;
    position_open_source_file(s->positions, name);
}


void scanner_close_stream(scanner_p s, void *stream)
// ----------------------------------------------------------------------------
//   Close the given input stream
// ----------------------------------------------------------------------------
{
    assert(stream == s->stream && "Only the current input can be closed");
    s->stream = NULL;
    s->reader = NULL;
}



// ============================================================================
//
//    Scanner implemmentation
//
// ============================================================================

static int scanner_getchar(scanner_p s)
// ----------------------------------------------------------------------------
//   Read next character from scanner
// ----------------------------------------------------------------------------
{
    char c = s->pending_char[0];
    if (c)
    {
        s->pending_char[0] = s->pending_char[1];
        s->pending_char[1] = 0;
        return c;
    }
    if (!s->reader)
        return EOF;
    unsigned size = s->reader(s->stream, 1, &c);
    if (size != 1)
    {
        s->reader = NULL;
        return EOF;
    }
    return c;
}


static inline void scanner_ungetchar(scanner_p s, char c)
// ----------------------------------------------------------------------------
//   Unget last character from input stream
// ----------------------------------------------------------------------------
{
    assert(s->pending_char[1] == 0 && "Max two pending char at a time");
    s->pending_char[1] = s->pending_char[0];
    s->pending_char[0] = c;
}


static inline unsigned scanner_position(scanner_p s)
// ----------------------------------------------------------------------------
//   Return the current position, taking into account chars we returned
// ----------------------------------------------------------------------------
{
    return position(s->positions)
        - (s->pending_char[0] != 0)
        - (s->pending_char[1] != 0);
}


static inline void scanner_consume(scanner_p s, char c)
// ----------------------------------------------------------------------------
//   Update position and token input after consuming one character
// ----------------------------------------------------------------------------
{
    if (c)
        text_append_data(&s->source, 1, &c);
    position_step(s->positions);
}


static inline int scanner_nextchar(scanner_p s, char c)
// ----------------------------------------------------------------------------
//   Consume current character and get the next one
// ----------------------------------------------------------------------------
{
    scanner_consume(s, c);
    return scanner_getchar(s);
}


static name_r scanner_normalize(text_p input)
// ----------------------------------------------------------------------------
//   Create an output name that is the normalized variant of the input
// ----------------------------------------------------------------------------
//   For normalization, we convert everything to lowercase and skip '_' chars
{
    const char *src = text_data(input);
    unsigned size = text_length(input);
    assert(name_is_valid(size, src) && "Normalizing invalid name");

    // Check for the relatively frequent case where input is already normalized
    // This happens for example with 'keywords' such as 'if' or 'then
    // in the way most people write code.
    bool normalized = true;
    unsigned normalized_size = 0;
    for (unsigned i = 0; i < size; i++)
    {
        char c = src[i];
        bool relevant = c != '_';
        normalized = relevant && c == tolower(c);
        normalized_size += relevant;
    }
    if (normalized)
        return (name_r) input;  // Assumes identical internal representations

    // It's not normalized. We need a new name to copy data into
    name_r result = name_new(text_position(input), normalized_size, src);
    char *dst = (char *) name_data(result);
    for (unsigned i = 0; i < size; i++)
    {
        char c = src[i];
        if (c == '_')
            continue;
        *dst++ = tolower(c);
    }
    return result;
}


token_t scanner_read(scanner_p s)
// ----------------------------------------------------------------------------
//    Scan input and return current token
// ----------------------------------------------------------------------------
{
    srcpos_t pos = scanner_position(s);

    // Clear source and text if any
    text_dispose(&s->source);
    text_dispose(&s->scanned.text);

    // Create new source text
    text_set(&s->source, text_new(pos, 0, NULL));

    // Check if we have something to read
    if (!s->reader)
        return tokEOF;

    // Check if we unindented far enough for multiple indents
    s->had_space_before = true;
    if (indents_length(s->indents) > 0 && indents_top(s->indents) > s->indent)
    {
        indents_pop(&s->indents);
        return tokUNINDENT;
    }

    // Read next character
    int c = scanner_getchar(s);

    // Skip spaces and check indendation
    s->had_space_before = false;
    while (isspace(c) && c != EOF)
    {
        s->had_space_before = true;
        if (c == '\n')
        {
            // New line: start counting indentation
            s->checking_indent = true;
            s->column = 0;
        }
        else if (s->checking_indent)
        {
            // Can't mix tabs and spaces
            if (c == ' ' || c == '\t')
            {
                if (!s->indent_char)
                    s->indent_char = c;
                else if (s->indent_char != c)
                    error(pos, "Mixed tabs and spaces in indentation");
            }
            s->column++;
        }

        // Keep looking for more spaces
        scanner_consume(s, c == '\n' ? c : 0);
        c = scanner_getchar(s);
    } // End of space processing (indentation check and space skipping)

    // Stop counting indentation
    if (s->checking_indent)
    {
        scanner_ungetchar(s, c);
        s->checking_indent = false;

        if (s->setting_indent)
        {
            // We set a new indent, for instance after opening paren
            indents_push(&s->indents, s->indent);
            s->indent = s->column;
            s->setting_indent = false;
            return tokNEWLINE;
        }
        else if (s->column > s->indent)
        {
            // Strictly deeper indent : report
            s->indent = s->column;
            indents_push(&s->indents, s->indent);
            return tokINDENT;
        }
        else if (s->column < indents_top(s->indents))
        {
            // Unindenting: remove rightmost indent level
            indents_pop(&s->indents);
            s->indent = s->column;

            // If we unindented, but did not go as far as the
            // most recent indent, report inconsistency.
            if (indents_length(s->indents) &&
                indents_top(s->indents) < s->column)
            {
                error(pos, "Unindenting to the right of previous indentation");
                return tokERROR;
            }

            // Otherwise, report that we unindented
            // We may report multiple tokUNINDENT if we unindented deep
            return tokUNINDENT;
        }
        else
        {
            // Exactly the same indent level as before
            return tokNEWLINE;
        }
    }

    // Report end of input if that's what we've got at that sage
    if (!s->reader)
	return tokEOF;

    // Clear spelling from whitespaces
    text_range(&s->source, 0, 0);

    // Update position to match first non-space
    pos = scanner_position(s);

    // Check if we have a blob
    blob_p blob = NULL;
    if (c == '$')
    {
        c = scanner_nextchar(s, c);
        blob_set(&blob, blob_new(pos, 0, NULL));
    }

    // Look for numbers
    if (blob || isdigit(c))
    {
        unsigned           base           = 10;
        unsigned           blob_base      = 16;
        unsigned long long natural_value  = 0;
        bool               floating_point = false;
        bool               based_number   = false;
        uint32_t           blob_chunk     = 0;
        unsigned           blob_bits      = 0;
        unsigned           blob_digbits   = 4;
        unsigned           blob_maxbits   = 8;

        // Initialize digit values the first time
        static uint8_t base_value[0x100] = { 0 };
        static uint8_t base64_value[0x100] = { 0 };
        if (base_value[0] == 0)
        {
            // For bases 2-36
            for (unsigned i = 0; i < 0x100; i++)
                base_value[i] = 0xFF;
            for (unsigned i = '0'; i <= '9'; i++)
                base_value[i] = i - '0';
            for (unsigned i = 'A'; i <= 'Z'; i++)
                base_value[i] = i - 'A' + 10;
            for (unsigned i = 'a'; i <= 'z'; i++)
                base_value[i] = i - 'a' + 10;

            // For base-64 (see https://en.wikipedia.org/wiki/Base64)
            for (unsigned i = 0; i < 0x100; i++)
                base64_value[i] = 0xFF;
            for (unsigned i = 'A'; i <= 'Z'; i++)
                base64_value[i] = i - 'A';
            for (unsigned i = 'a'; i <= 'z'; i++)
                base64_value[i] = i - 'a' + 26;
            for (unsigned i = '0'; i <= '9'; i++)
                base64_value[i] = i - '0' + 52;
            base64_value['+'] = 62;
            base64_value['/'] = 63;
        }

        uint8_t *digit_value = base_value;

        // Take integral part (or base)
        do
        {
            while (digit_value[c] < base ||
                   (blob && digit_value[c] < blob_base))
            {
                natural_value = base * natural_value + digit_value[c];
                if (blob)
                {
                    // Record blob digits as we go
                    blob_chunk = (blob_chunk << blob_digbits) | digit_value[c];
                    blob_bits += blob_digbits;
                    if (blob_bits >= blob_maxbits)
                    {
                        if (blob_maxbits == 8)
                        {
                            char blob_byte = blob_chunk;
                            blob_append_data(&blob, 1, &blob_byte);
                        }
                        else
                        {
                            char blob_bytes[3] = { (char) (blob_chunk >> 16),
                                                   (char) (blob_chunk >> 8),
                                                   (char) blob_chunk };
                            blob_append_data(&blob, 3, blob_bytes);
                        }
                    }

                }
                c = scanner_nextchar(s, c);
                if (c == '_')       // Skip a single underscore
                {
                    c = scanner_nextchar(s, c);
                    if (c == '_')
                        error(pos, "Two '_' characters in a row look ugly");
                }
                if (blob)
                {
                    // Skip whitespace in blobs
                    while (c != EOF && isspace(c))
                        c = scanner_nextchar(s, 0);
                }

            }

            // Check if this is a based number or blob
            if (c == '#' && !based_number)
            {
                base = blob_base = natural_value;
                if (base == 64)
                {
                    // Special case for base-64: switch coding table
                    digit_value = base64_value;
                }
                else if (base < 2 || base > 36)
                {
                    base = 36;
                    error(pos, "The base %d is not valid, not in 2..36", base);
                }
                else if (blob)
                {
                    // Remove any byte we may have recorded in the blob
                    blob_range(&blob, 0, 0);
                    blob_bits = 0;
                    blob_chunk = 0;

                    // Select bit sizes depending on base
                    switch(base)
                    {
                    case 2:  blob_digbits = 1; break;
                    case 4:  blob_digbits = 2; break;
                    case 8:  blob_digbits = 3; blob_maxbits = 24; break;
                    case 16: blob_digbits = 4; break;
                    case 64: blob_digbits = 6; blob_maxbits = 24; break;
                    default:
                        error(pos, "Base %d is invalid for a blob", base);
                    }
                }
                c = scanner_nextchar(s, c);
                natural_value = 0;
                based_number = true;
            }
            else
            {
                based_number = false;
            }
        } while (based_number);
        double real_value = natural_value;

        // Check if there is still something to record in the blob
        if (blob)
        {
            // Check = terminator in base64
            if (blob_base == 64 && c == '=')
                c = scanner_nextchar(s, c);

            // Check if there is a $ at end of blob
            if (c == '$')
                scanner_consume(s, c);
            else
                scanner_ungetchar(s, c);

            if (blob_bits)
            {
                // Pad with 0 as necessary
                while (blob_bits < blob_maxbits)
                {
                    blob_chunk <<= blob_digbits;
                    blob_bits += blob_digbits;
                }

                // Record last byte / chunk in the blob
                if (blob_maxbits == 8)
                {
                    char blob_byte = blob_chunk;
                    blob_append_data(&blob, 1, &blob_byte);
                }
                else
                {
                    char blob_bytes[3] = { (char) (blob_chunk >> 16),
                                           (char) (blob_chunk >> 8),
                                           (char) blob_chunk };
                    blob_append_data(&blob, 3, blob_bytes);
                }
            }
            blob_set(&s->scanned.blob, (blob_r) blob);
            return tokBLOB;
        }

        // Check for fractional part for real numbers
        else if (c == '.')
        {
            int mantissa_digit = scanner_nextchar(s, c);
            if (digit_value[mantissa_digit] >= base)
            {
                // This is something else following an integer: 1..3, 1.(3)
                natural_r n = natural_new(pos, natural_value);
                scanner_ungetchar(s, mantissa_digit);
                scanner_ungetchar(s, c);
                s->had_space_after = false;
                natural_set(&s->scanned.natural, n);
                return tokINTEGER;
            }
            else
            {
                double comma_position = 1.0;
                floating_point = true;
                c = mantissa_digit;
                while (digit_value[c] < base)
                {
                    comma_position /= base;
                    real_value += comma_position * digit_value[c];
                    c = scanner_nextchar(s, c);
                    if (c == '_')
                    {
                        c = scanner_nextchar(s, c);
                        if (c == '_')
                            error(pos, "Two _ characters look really ugly");
                    }
                }
            }
        }

        // Check if we have a second '#' at end of based number (16#FF#e3)
        if (c == '#')
            c = scanner_nextchar(s, c);

        // Check for the exponent
        if (c == 'e' || c == 'E')
        {
            c = scanner_nextchar(s, c);

            unsigned exponent = 0;
            bool negative_exponent = false;

            // Exponent sign
            if (c == '+')
            {
                c = scanner_nextchar(s, c);
            }
            else if (c == '-')
            {
                c = scanner_nextchar(s, c);
                negative_exponent = true;
                floating_point = true;
            }

            // Exponent value (always in base 10)
            while (base_value[c] < 10)
            {
                exponent = 10 * exponent + base_value[c];
                c = scanner_nextchar(s, c);
                if (c == '_')
                    c = scanner_nextchar(s, c);
            }

            // Compute base^exponent
            if (floating_point)
            {
                double exponent_value = 1.0;
                double multiplier = base;
                while (exponent)
                {
                    if (exponent & 1)
                        exponent_value *= multiplier;
                    exponent >>= 1;
                    multiplier *= multiplier;
                }

                // Compute actual value
                if (negative_exponent)
                    real_value /= exponent_value;
                else
                    real_value *= exponent_value;
            }
            else
            {
                unsigned long long exponent_value = 1;
                unsigned long long multiplier = base;
                while (exponent)
                {
                    if (exponent & 1)
                        exponent_value *= multiplier;
                    exponent >>= 1;
                    multiplier *= multiplier;
                }
                natural_value *= exponent_value;
            }
        }

        // Return the token
        scanner_ungetchar(s, c);
        s->had_space_after = isspace(c);
        if (floating_point)
        {
            real_set(&s->scanned.real, real_new(pos, real_value));
            return tokREAL;
        }
        natural_set(&s->scanned.natural, natural_new(pos, natural_value));
        return tokINTEGER;
    } // End of numbers

    // Look for names
    else if (utf8_isalpha(c))
    {
        while (isalnum(c) || c == '_' || utf8_is_first(c) || utf8_is_next(c))
            c = scanner_nextchar(s, c);
        scanner_ungetchar(s, c);
        s->had_space_after = isspace(c);

        // Check if this is a block marker
        name_set(&s->scanned.name,scanner_normalize(s->source));
        if (syntax_is_block_open(s->scanned.name))
            return tokOPEN;
        else if (syntax_is_block_close(s->scanned.name))
            return tokCLOSE;
        return tokNAME;
    } // End of names

    // Look for texts
    else if (c == '"' || c == '\'')
    {
        char eos = c;
        text_p text = text_new(pos, 0, NULL);
        c = scanner_nextchar(s, c);
        for(;;)
        {
            // Check end of text
            if (c == eos)
            {
                c = scanner_nextchar(s, c);
                if (c != eos)
                {
                    scanner_ungetchar(s, c);
                    s->had_space_after = isspace(c);
                    s->scanned.text = text;
                    return eos == '"' ? tokTEXT : tokCHARACTER;
                }

                // Double: put it in
            }
            if (c == EOF)
            {
                error(pos, "End of input in the middle of a text");
                s->had_space_after = false;
                text_set(&s->scanned.text, (text_r) text);
                return eos == '"' ? tokTEXT : tokCHARACTER;
            }
            char ch = (char) c;
            text_append_data(&text, 1, &ch);
            c = scanner_nextchar(s, c);
        }
    } // End of text handling

    // Look for single-char block delimiters (parentheses, etc)
    bool is_open = syntax_is_block_open_character(c);
    bool is_close = !is_open && syntax_is_block_close_character(c);
    if (is_open || is_close)
    {
        name_set(&s->scanned.name, scanner_normalize(s->source));
        s->had_space_after = false;
        return is_open ? tokOPEN : tokCLOSE;
    }

    // Look for other symbols
    while (ispunct(c) && c != '\'' && c != '"' && c != EOF &&
           !syntax_is_block_open_character(c) &&
           !syntax_is_block_close_character(c))
    {
        c = scanner_nextchar(s, c);
        if (!s->reading_syntax && !syntax_known(s->source))
            break;
    }
    scanner_ungetchar(s, c);
    s->had_space_after = isspace(c);
    name_set(&s->scanned.name, scanner_normalize(s->source));
    if (syntax_is_block_open(s->scanned.name))
        return tokOPEN;
    if (syntax_is_block_close(s->scanned.name))
        return tokCLOSE;
    return tokSYMBOL;
}


text_p scanner_comment(scanner_p s, name_p closing)
// ----------------------------------------------------------------------------
//    Read ahead until end of comment, save comment in 'source'
// ----------------------------------------------------------------------------
{
    const char *eoc      = name_data(closing);
    const char *match    = eoc;
    unsigned    position = scanner_position(s);
    text_p      comment  = text_new(position, 0, NULL);
    int         c        = 0;
    bool        skip     = false;

    // Clear source and scanned value if any
    text_dispose(&s->source);
    text_dispose(&s->scanned.text);

    while (*match && c != EOF)
    {
        c = scanner_nextchar(s, c);
        skip = false;

        if (c == '\n')
        {
            // New line: start counting indentation
            s->checking_indent = true;
            s->column = 0;
        }
        else if (s->checking_indent)
        {
            if (isspace(c))
            {
                skip = s->column < s->indent;
            }
            else
            {
                s->checking_indent = false;
                skip = false;
            }
        }

        if (c == *match)
        {
            match++;
        }
        else
        {
            // Backtrack in case we had something like **/
            unsigned max = match - eoc;
            const char *end = text_data(comment) + text_length(comment);
            for (unsigned i = 0; i < max; i++)
            {
                // Stop as soon as we start matching again
                if (memcmp(eoc, end - max + i, match - eoc) == 0)
                    break;
                match--;
            }
        }

        if (!skip)
        {
            char ch = (char) c;
            text_append_data(&comment, 1, &ch);
        }
    }

    // Strip the termination from the comment being returned
    size_t comment_length = text_length(comment) - (match - eoc);
    text_range(&comment, 0, comment_length);
    return comment;
}
