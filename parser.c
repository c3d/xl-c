// ****************************************************************************
//  parser.c                                        XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     ELFE language parser
//
//      See detailed description in parser.h
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

#include "parser.h"
#include "number.h"
#include "pfix.h"
#include "infix.h"
#include "block.h"
#include "delimited_text.h"



// ============================================================================
//
//    Creating and deleting a parser
//
// ============================================================================

parser_p parser_new(const char *filename,
                    positions_p positions,
                    syntax_p syntax)
// ----------------------------------------------------------------------------
//   Create a new parser
// ----------------------------------------------------------------------------
{
    scanner_p s = scanner_new(positions, syntax);
    scanner_open(s, filename);

    parser_p p = malloc(sizeof(parser_t));
    p->scanner = s;
    p->comment = NULL;
    p->pending = tokNONE;
    p->had_space_before = false;
    p->had_space_after = false;
    p->beginning_line = false;

    return p;
}


void parser_delete(parser_p p)
// ----------------------------------------------------------------------------
//    Delete a parser
// ----------------------------------------------------------------------------
{
    scanner_close(p->scanner, (FILE *) p->scanner->stream);
    scanner_delete(p->scanner);
    free(p);
}



// ============================================================================
//
//    Parsing XL input
//
// ============================================================================

typedef struct pending
// ----------------------------------------------------------------------------
//   A stack used to store the pending operations during parsing
// ----------------------------------------------------------------------------
// For example, when parsing A+B*C, it will hold (A, +) and (B, *)
// We push 3 trees for each level: the opcode, the argument and the priority
{
    name_p      opcode;         // The opcode, e.g. + or *
    tree_p      argument;       // The argument, e.g. A or B
    unsigned    priority;       // The priority, to know when to pop
} pending_t, *pending_p;

#define inline extern inline
blob_type(pending_t, pending_stack);
blob_type_handler(pending_stack);
#undef inline


static token_t parser_token(parser_p p)
// ----------------------------------------------------------------------------
//    Return next parser token, skipping comments and gathering long text
// ----------------------------------------------------------------------------
//    In some cases, we need to push back one 'pending' token
//    (not to be confused with 'pending' operations on the stack)
{
    scanner_p   scanner   = p->scanner;
    syntax_p    syntax    = scanner->syntax;
    name_p      opening   = NULL;
    name_p      closing   = NULL;

    while (true)
    {
        token_t pend = p->pending;
        if (pend != tokNONE && pend != tokNEWLINE)
        {
            p->pending = tokNONE;
            p->beginning_line = false;
            return pend;
        }

        // Here, there's nothing pending or only a newline
        token_t result = scanner_read(scanner);
        p->had_space_before = scanner->had_space_before;
        p->had_space_after = scanner->had_space_after;

        switch(result)
        {
        case tokNAME:
        case tokSYMBOL:
            opening = scanner->scanned.name;
            if (name_eq(opening, "syntax"))
            {
                syntax_read(scanner->syntax, scanner);
                continue;
            }
            else if (syntax_is_comment(syntax, opening, &closing))
            {
                // Skip comments, keep looking to get the right indentation
                text_p comment = scanner_skip(scanner, closing);
                if (p->comment)
                    text_append(&p->comment, comment);
                else
                    p->comment = comment;
                if (name_eq(closing, "\n") && pend == tokNONE)
                {
                    p->pending = tokNEWLINE;
                    p->beginning_line = true;
                }
                continue;
            }
            else if (syntax_is_text(syntax, opening, &closing))
            {
                text_p val = scanner_skip(scanner, closing);
                name_p op = (name_p) opening;
                name_p cl = (name_p) closing;
                srcpos_t pos = name_position(op);
                delimited_text_p dt = delimited_text_new(pos, val, op, cl);
                scanner->scanned.tree = (tree_p) dt;
                if (pend == tokNEWLINE)
                {
                    p->pending = tokLONGTEXT;
                    return tokNEWLINE;
                }
                if (name_eq(closing, "\n") && pend == tokNONE)
                {
                    p->pending = tokNEWLINE;
                    p->beginning_line = true;
                }
                else
                {
                    p->beginning_line = false;
                }
                return tokLONGTEXT;
            }

            // If the next token has a substatement infix priority,
            // this takes over any pending newline. Example: else
            if (pend == tokNEWLINE)
            {
                int prefixPrio = syntax_prefix_priority(syntax, opening);
                if (prefixPrio == syntax->default_priority)
                {
                    int infixPrio = syntax_infix_priority(syntax, opening);
                    if (infixPrio < syntax->statement_priority)
                        p->pending = pend = tokNONE;
                }
            }

            // All comments after this will be following the token
            p->beginning_line = false;
            break;

        case tokNEWLINE:
            // Combine newline with any previous pending indent
            p->pending = tokNEWLINE;
            p->beginning_line = true;
            continue;

        case tokUNINDENT:
            // Add newline if what comes next isn't an infix like 'else'
            p->pending = tokNEWLINE;
            p->beginning_line = true;
            return result;
        case tokINDENT:
            // If we had a new-line followed by indent, ignore the new line
            p->pending = tokNONE;
            p->beginning_line = true;
            return result;
        default:
            p->beginning_line = false;
            break;
        } // switch (result)

        // If we have another token here and a pending newline, push
        // the other token back.
        if (pend != tokNONE)
        {
            p->pending = result;
            p->beginning_line = true;
            return pend;
        }

        return result;
    } // While loop
}


static inline tree_p parser_prefix_new(name_p left, tree_p right)
// ----------------------------------------------------------------------------
//   Create a prefix, special-case unary minus with constants
// ----------------------------------------------------------------------------
{
    if (name_eq(left, "-"))
    {
        natural_p n = natural_cast(right);
        if (n)
            return (tree_p) integer_new(natural_position(n), -natural_value(n));

        integer_p i = integer_cast(right);
        if (i)
        {
            i->value = -i->value;
            return (tree_p) i;
        }

        real_p r = real_cast(right);
        if (r)
        {
            r->value = -r->value;
            return (tree_p) r;
        }
    }
    return (tree_p) prefix_new(name_position(left), left, right);
}


static inline tree_p parser_pfix_new(tree_p left, tree_p right)
// ----------------------------------------------------------------------------
//    If left is a name, create a prefix, else a pfix
// ----------------------------------------------------------------------------
{
    name_p name = name_cast(left);
    if (name)
        return parser_prefix_new(name, right);
    return (tree_p) pfix_new(tree_position(left), left, right);
}


static tree_p parser_block(parser_p p,
                           name_p   block_opening,
                           name_p   block_closing,
                           int      block_priority)
// ----------------------------------------------------------------------------
//    Parse input until we reach block_end
// ----------------------------------------------------------------------------
// XL parsing is not very difficult, but a bit unusual, because it is based
// solely on dynamic information and not, for instance, on keywords.
// Consider the following cases, where p is "prefix-op" and i is "infix-op"
//     Write A
//       Parses as p(Write,A).
//     A and B
//       Parses as i(and,A,B) if 'and' has a priority,
//              as p(A,p(and,B)) otherwise
//     Write -A,B
//       This parses as (Write-A),B since "-" has a priority.
//       This one is fixed "artificially" by using spaces around -
//       If there is a space before but not after, we parse as
//           Write (-A), B
{
    typedef pending_stack_p stack_p;

    tree_p      result             = NULL;
    tree_p      left               = NULL;
    tree_p      right              = NULL;
    scanner_p   scanner            = p->scanner;
    syntax_p    syntax             = syntax_use(scanner->syntax);
    positions_p positions          = scanner->positions;
    srcpos_t    pos                = position(positions);
    int         default_priority   = syntax->default_priority;
    int         function_priority  = syntax->function_priority;
    int         statement_priority = syntax->statement_priority;
    int         result_priority    = default_priority;
    int         prefix_priority    = 0;
    int         prefix_vs_infix    = 0;
    int         postfix_priority   = 0;
    int         infix_priority     = 0;
    unsigned    old_indent         = 0;
    name_p      infix              = NULL;
    name_p      name               = NULL;
    name_p      opening            = NULL;
    name_p      closing            = NULL;
    block_p     block              = NULL;
    stack_p     stack              = pending_stack_new(pos, 0, NULL);
    syntax_p    child_syntax       = NULL;
    name_p      child_syntax_end   = NULL;
    token_t     tok                = tokNONE;
    bool        is_expression      = false;
    bool        new_statement      = true;
    bool        done               = false;

#define STACK_PUSH(op, arg, prio)                       \
    do                                                  \
    {                                                   \
        pending_t pending = { (op), (arg), (prio) };    \
        if (pending.opcode)                             \
            name_ref(pending.opcode);                   \
        tree_ref(pending.argument);                     \
        pending_stack_push(&stack, pending);            \
    } while (0)

    // Check priorities compared to stack
    // A + B * C, we got '*': keep "A+..." on stack
    // Odd priorities are made right-associative by turning the
    // low-bit off (with ~1) in the comparison below
#define STACK_FLUSH()                                                   \
    do                                                                  \
    {                                                                   \
        while (pending_stack_length(stack))                             \
        {                                                               \
            pending_t prev = pending_stack_top(stack);                  \
                                                                        \
            if (!done &&                                                \
                prev.priority != default_priority &&                    \
                infix_priority > (prev.priority & ~1))                  \
                break;                                                  \
            if (prev.opcode == NULL) /* Prefix */                       \
            {                                                           \
                tree_set(&result,                                       \
                         parser_pfix_new(prev.argument, result));       \
            }                                                           \
            else                                                        \
            {                                                           \
                tree_set(&result, (tree_p)                              \
                         infix_new(name_position(prev.opcode),          \
                                   prev.opcode,                         \
                                   prev.argument,                       \
                                   result));                            \
                name_dispose(&prev.opcode);                             \
            }                                                           \
            tree_dispose(&prev.argument);                               \
            pending_stack_pop(&stack);                                  \
        }                                                               \
    } while(0)

    if (block_opening)
    {
        assert(block_closing && "Block needs opening and closing");
        assert(syntax_infix_priority(syntax, block_opening) == block_priority);
        assert(syntax_infix_priority(syntax, block_closing) == block_priority);

        // We are creating a block for everything inside
        block = block_use(block_new(pos, block_opening, block_closing));

        // When inside a () block, we are in 'expression' mode right away
        if (block_priority > statement_priority)
        {
            new_statement = false;
            is_expression = true;
        }
    }

    while (!done)
    {
        // Scan next token
        tree_dispose(&right);
        prefix_priority = infix_priority = default_priority;
        tok = parser_token(p);

        // Check token result
        switch(tok)
        {
        case tokEOF:
        case tokERROR:
            done = true;
            if (block && !name_eq(block_closing, SYNTAX_UNINDENT))
                error(pos,
                      "Unexpected end of text, expected %t to close block",
                      block_closing);
            break;
        case tokINTEGER:
        case tokREAL:
        case tokCHARACTER:
        case tokTEXT:
        case tokLONGTEXT:
            tree_set(&right, scanner->scanned.tree);
            if (!result && new_statement)
                is_expression = false;
            prefix_priority = function_priority;
            break;

        case tokNEWLINE:
            // Consider new-line as an infix operator
            name_set(&scanner->scanned.name, name_cnew(pos, "\n"));
            // Intentionally Fall-through

        case tokNAME:
        case tokSYMBOL:
            name_set(&name, scanner->scanned.name);
            if (block && name_compare(name, block_closing) == 0)
            {
                done = true;
                break;
            }
            else if ((child_syntax = syntax_is_special(syntax, name,
                                                       &child_syntax_end)))
            {
                // Read the input with the special syntax
                int prio = syntax_infix_priority(syntax, name);
                scanner->syntax = child_syntax;
                right = parser_block(p, name, child_syntax_end, prio);
                scanner->syntax = syntax;
            }
            else if (!result)
            {
                prefix_priority = syntax_prefix_priority(syntax, name);
                tree_set(&right, (tree_p) name);
                if (prefix_priority == default_priority)
                    prefix_priority = function_priority;
                if (new_statement && tok == tokNAME)
                    is_expression = false;
            }
            else if (left)
            {
                // This is the right of an infix operator
                // If we have "A and not B", where "not" has
                // higher priority than "and", we want to
                // parse this as "A and (not B)" rather than as
                // "(A and not) B"
                prefix_priority = syntax_prefix_priority(syntax, name);
                tree_set(&right, (tree_p) name);
                if (prefix_priority == default_priority)
                    prefix_priority = function_priority;
            }
            else
            {
                // Complicated case: need to discriminate infix and prefix
                infix_priority = syntax_infix_priority(syntax, name);
                prefix_vs_infix = syntax_prefix_priority(syntax, name);
                if (infix_priority != default_priority &&
                    (prefix_vs_infix == default_priority ||
                     !p->had_space_before || p->had_space_after))
                {
                    // If this infix matches the current block, append it
                    if (block)
                    {
                        // Check that we have consistent separators within block
                        if (!block->separator)
                        {
                            block->separator = name_use(name);
                        }
                        else if (name_compare(block->separator, name) != 0)
                        {
                            error(pos, "Inconsistent separator in block: "
                                  "had %t, now %t", block->separator, name);
                            error(name_position(block->separator),
                                  "This is where separator %t was found",
                                  block->separator);
                        }

                        // Append to block
                        block_append_data(&block, 1, &result);
                        tree_set(&result, (tree_p) block);
                    }
                    else
                    {
                        // We got an infix
                        tree_set(&left, result);
                        name_set(&infix, name);
                    }
                }
                else
                {
                    postfix_priority = syntax_postfix_priority(syntax, name);
                    if (postfix_priority != default_priority)
                    {
                        // We have a postfix operator
                        tree_set(&right, (tree_p) name);

                        // Flush higher priority items on stack
                        // This is the case for X:integer!
                        STACK_FLUSH();

                        tree_set(&right, (tree_p)
                                 postfix_new(pos, result, (name_p) right));
                        prefix_priority = postfix_priority;
                        tree_dispose(&result);
                    }
                    else
                    {
                        // No priority: take this as a prefix by default
                        tree_set(&right, (tree_p) name);
                        prefix_priority = prefix_vs_infix;
                        if (prefix_priority == default_priority)
                        {
                            prefix_priority = function_priority;
                            if (new_statement && tok == tokNAME)
                                is_expression = false;
                        }
                    }
                }
            }
            break;
        case tokCLOSE:
            // Check for mismatched parenthese here
            if (name_compare(scanner->scanned.name, block_closing) != 0)
                error(pos, "Mismatched parentheses: got %t, expected %t",
                      scanner->scanned.name, block_closing);
            done = true;
            break;
        case tokUNINDENT:
            // Check for mismatched blocks here
            if (!name_eq(block_closing, SYNTAX_UNINDENT))
                error(pos, "Mismatched identation, expected %t", block_closing);
            done = true;
            break;
        case tokINDENT:
            name_set(&scanner->scanned.name, name_cnew(pos, SYNTAX_INDENT));
            // Intentionally fall-through

        case tokOPEN:
            name_set(&opening, scanner->scanned.name);
            if (!syntax_is_block(syntax, opening, &closing))
                error(pos, "Unknown parenthese type %t", opening);
            if (tok == tokOPEN)
                old_indent = scanner_open_parenthese(scanner);
            prefix_priority = syntax_infix_priority(syntax, block_opening);

            // Just like for names, parse the contents of the parentheses
            infix_priority = default_priority;
            right = parser_block(p, opening, closing, prefix_priority);
            if (tok == tokOPEN)
                scanner_close_parenthese(scanner, old_indent);
            break;
        default:
            error(pos, "Unnknown token for %t, value %u", scanner->source, tok);
            break;
        } // switch(tok)


        // Check what is the current result
        if (!result)
        {
            // First thing we parse
            tree_set(&result, right);
            result_priority = prefix_priority;

            // We are now in the middle of an expression
            if (result && result_priority >= statement_priority)
                new_statement= false;
        }
        else if (left)
        {
            // Check if we had a statement separator
            if (infix_priority < statement_priority)
            {
                new_statement = true;
                is_expression = false;
            }

            // We got left and infix-op, we are now looking for right
            // If we have 'A and not B', where 'not' has higher priority
            // than 'and', we want to finish parsing 'not B' first, rather
            // than seeing this as '(A and not) B'.
            if (prefix_priority != default_priority)
            {
                // Push "A and" in the above example
                STACK_PUSH(infix, left, infix_priority);
                left = NULL;

                // Start over with "not"
                tree_set(&result, right);
                result_priority = prefix_priority;
            }
            else
            {
                STACK_FLUSH();

                // Now, we want to restart with the rightmost operand
                if (done)
                {
                    // End of text: the result is what we just got
                    tree_set(&result, left);
                }
                else
                {
                    // Something like A+B+C, just got second +
                    STACK_PUSH(infix, left, infix_priority);
                    result = NULL;
                }
                tree_dispose(&left);
            }
        }
        else if (right)
        {
            // Check if we had a low-priority prefix (e.g. pragmas)
            if (prefix_priority < statement_priority)
            {
                new_statement = true;
                is_expression = false;
            }

            // Check priorities for something like "A.B x,y" -> "(A.B) (x,y)"
            // Odd priorities are made right associative by turning the
            // low bit off for the previous priority
            if (prefix_priority <= result_priority)
            {
                STACK_FLUSH();
            }

            // Check if new statement
            if (!is_expression)
                if (result_priority > statement_priority)
                    if (pending_stack_length(stack) == 0 ||
                        pending_stack_top(stack).priority < statement_priority)
                        result_priority = statement_priority;

            // Push a recognized prefix op
            STACK_PUSH(NULL, result, result_priority);
            tree_set(&result, right);
            result_priority = prefix_priority;
        }

        // Retrieve the position for the next round
        pos = position(positions);
    } // While(!done)

    if (pending_stack_length(stack))
    {
        if (!result)
        {
            pending_t last = pending_stack_top(stack);
            if (last.opcode && !name_eq(last.opcode, "\n"))
                tree_set(&result, (tree_p)
                         postfix_new(pos, last.argument, last.opcode));
            else
                tree_set(&result, last.argument);
            pending_stack_pop(&stack);
        }

        // Check if some stuff remains on stack
        STACK_FLUSH();
    }

    pending_stack_dispose(&stack);

    return result;
}


tree_p parser_parse(parser_p p)
// ----------------------------------------------------------------------------
//   Parse input from the given parser
// ----------------------------------------------------------------------------
{
    return parser_block(p, NULL, NULL, 0);
}
