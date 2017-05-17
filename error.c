// ****************************************************************************
//  error.cpp                                       XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Error management in the XL compiler
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

#include "error.h"
#include "text.h"
#include "blob.h"
#include "position.h"
#include "array.h"
#include "position.h"
#include "array.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>


// Silence warning about unused inline functions defined in .c file
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
array_typedef(text, errors);
#pragma GCC diagnostic pop

static errors_p    errors    = NULL;
static positions_p positions = NULL;



// ============================================================================
//
//    Display errors
//
// ============================================================================

static void error_display(text_p error)
// ----------------------------------------------------------------------------
//    Display the error
// ----------------------------------------------------------------------------
{
    // Retrieve file / line information from position
    srcpos_t pos = text_position(error);
    position_t posinfo = { 0 };
    bool ok = position_info(positions, pos, &posinfo);

    // Printout the message
    if (ok)
    {
        fprintf(stderr, "%s:%d: %*s\n",
                posinfo.file, posinfo.line,
                (int) text_length(error), text_data(error));

        // Retrieve source code
        size_t size = posinfo.line_length + 1;
        char *buffer = malloc(size);
        ok = position_source(&posinfo, buffer, size);
        if (ok)
        {
            fprintf(stderr, "  %s\n", buffer);

            // Display caret to show column position
            int col = (int) posinfo.column;
            fprintf(stderr, "  %*s^\n", col, "");
        }
        free(buffer);
    }
    else
    {
        fprintf(stderr, "<unknown position>: %*s\n",
                (int) text_length(error), text_data(error));
    }
}


static void errors_display(errors_p *errors)
// ----------------------------------------------------------------------------
//   Display all errors in a saved errors list and clear it
// ----------------------------------------------------------------------------
{
    size_t count = errors_length(*errors);
    text_p *errs = errors_data(*errors);
    for (size_t e = 0; e < count; e++)
        error_display(errs[e]);
    errors_dispose(errors);
}



// ============================================================================
//
//    Create errors
//
// ============================================================================

void error(srcpos_t position, const char *message, ...)
// ----------------------------------------------------------------------------
//    Format the error message and record it
// ----------------------------------------------------------------------------
{
    va_list va;
    va_start(va, message);
    errorv(position, message, va);
    va_end(va);
}


void errorv(srcpos_t position, const char *message, va_list va)
// ----------------------------------------------------------------------------
//    Process the error message
// ----------------------------------------------------------------------------
{
    text_r err = text_vprintf(position, message, va);
    if (errors)
        errors = errors_push(errors, err);
    else
        error_display(err);
}



// ============================================================================
//
//    Error positions
//
// ============================================================================

positions_p error_positions()
// ----------------------------------------------------------------------------
//   Return current positions records for errors
// ----------------------------------------------------------------------------
{
    return positions;
}


positions_p error_set_positions(positions_p new_pos)
// ----------------------------------------------------------------------------
//    Set positions records for errors, return old one
// ----------------------------------------------------------------------------
{
    positions_p old = positions;
    positions = new_pos;
    return old;
}



// ============================================================================
//
//    Hierarchical error contexts
//
// ============================================================================

errors_p errors_save()
// ----------------------------------------------------------------------------
//    Create a new error context, return old one
// ----------------------------------------------------------------------------
{
    errors_p result = errors;
    srcpos_t position = positions ? positions->position : 0;
    errors = errors_use(errors_new(position, 0, NULL));
    return result;
}


void errors_commit(errors_p saved_errors)
// ----------------------------------------------------------------------------
//    Accept errors in the current error context
// ----------------------------------------------------------------------------
{
    if (saved_errors)
    {
        // Append errors to previous ones
        saved_errors = errors_append(saved_errors, errors);
        errors_dispose(&errors);
        errors = saved_errors;
    }
    else
    {
        // Display errors immediately and discard them
        errors_display(&errors);
    }
}


void errors_clear(errors_p saved_errors)
// ----------------------------------------------------------------------------
//    Discard errors in the current errors list, restore old one
// ----------------------------------------------------------------------------
{
    errors_dispose(&errors);
    errors = saved_errors;
}


unsigned errors_count()
// ----------------------------------------------------------------------------
//   Return the number of errors in the current error list
// ----------------------------------------------------------------------------
{
    assert(errors && "Cannot count errors if not recording them");
    return errors_length(errors);
}
