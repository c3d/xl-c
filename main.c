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
#include "number.h"
#include "name.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char *x = "Hello world";
    integer_p n = integer_new(0, 42);
    name_p name = name_new(0, 1, "X");
    text_p t = text_printf(0,
                           "Name X is %t\n"
                           "The value of 42 is %t (%d).\n"
                           "The value of x is %p (%s)\n", name, n, 42, x, x);
    text_print(stdout, t);
    text_dispose(&t);
    return 0;
}
