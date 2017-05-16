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

#include "text.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    text_p t = text_use(text_cnew(0, "Hello world"));
    text_print(stdout, t);
    text_dispose(&t);
    return 0;
}
