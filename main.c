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
#include "renderer.h"
#include "text.h"

#include <stdio.h>
#include <string.h>


RECORDER(MAIN, 32, "Main function");

int main(int argc, char *argv[])
{
    RECORD(MAIN, "Starting %s with %d args", argv[0], argc);
    recorder_dump_on_common_signals(0,0);

#ifndef PREFIX_PATH
#define PREFIX_PATH  "/Users/ddd/Work/xl/"
#endif

    positions_p positions = positions_new();
    error_set_positions(positions);

    renderer_p renderer = renderer_new(PREFIX_PATH "xl.stylesheet");
    error_set_renderer(renderer);

    syntax_p syntax = syntax_use(syntax_new(PREFIX_PATH "xl.syntax"));
    for (int arg = 1; arg < argc; arg++)
    {
        parser_p parser = parser_new(argv[arg], positions, syntax);
        tree_p tree = tree_use(parser_parse(parser));
        fprintf(stderr, "File #%d: %s: ", arg, argv[arg]);
        tree_print(stderr, tree);
        parser_delete(parser);
        tree_dispose(&tree);
    }

    syntax_dispose(&syntax);
    renderer_delete(renderer);
    positions_delete(positions);

    renderer_p last_renderer = renderer_new(NULL);
    error_set_renderer(last_renderer);
    tree_memcheck(0);
    renderer_delete(last_renderer);

    return 0;
}
