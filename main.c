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
#include "parser.h"
#include "position.h"
#include "recorder.h"
#include "text.h"

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    RECORD(MAIN, "Starting %s with %d args", argv[0], argc);

#ifndef PREFIX_PATH
#define PREFIX_PATH  "/Users/ddd/Work/xl/"
#endif

    positions_p positions = positions_new();
    syntax_p syntax = syntax_new(PREFIX_PATH "xl.syntax");
    for (int arg = 1; arg < argc; arg++)
    {
        parser_p parser = parser_new(argv[arg], positions, syntax);
        tree_p tree = parser_parse(parser);
        tree_print(stdout, tree);
        parser_delete(parser);
        tree_dispose(&tree);
    }
    syntax_dispose(&syntax);
    positions_delete(positions);

    tree_memcheck();
    return 0;
}
