// ****************************************************************************
//  main.c                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//
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
#include "name.h"
#include "number.h"
#include "position.h"
#include "recorder.h"
#include "text.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    RECORD(MAIN, "Starting %s with %d args", argv[0], argc);

    char *x = "Hello world";
    integer_p n = integer_new(0, 42);
    name_p name = name_new(0, 1, "X");
    text_p t = text_printf(0,
                           "Name X is %t\n"
                           "The value of 42 is %t (%d).\n"
                           "The value of x is %p (%s)\n", name, n, 42, x, x);
    text_print(stdout, t);

    positions_p positions = positions_new();
    error_set_positions(positions);

    position_open_source_file(positions, "/Users/ddd/Work/xl/main.c");
    error(766, "The position of main should be correct at %d", 766);

    for (int commit = 0; commit <= 1; commit++)
    {
        errors_p errs = errors_save();
        error(1471, "This error should be shown if commit %d is 1", commit);
        error(1548, "The value of error pointer is %p", errs);
        if (commit)
            errors_commit(errs);
        else
            errors_clear(errs);
    }

    text_dispose(&t);
    name_dispose(&name);
    integer_dispose(&n);

    error_set_positions(NULL);
    error(0,
          "The variable %t does not have value %t (%f * 3)", name, n, 42.0/3);

    positions_delete(positions);
    tree_memcheck();
    return 0;
}
