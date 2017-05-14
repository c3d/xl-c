// ****************************************************************************
//  number.c                                      XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of numbers in the XL compiler
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

#include "number.h"
#include <wchar.h>
#include <stdlib.h>
#include <string.h>


#define NUMBER(number, printf_format, reptype, va_type)                 \
                                                                        \
tree_p number##_handler(tree_cmd_t cmd, tree_p tree, va_list va)        \
{                                                                       \
    number##_p     number = (number##_p) tree;                          \
    size_t        size;                                                 \
    tree_io_fn    io;                                                   \
    reptype       value;                                                \
    void *        stream;                                               \
    char          buffer[32];                                           \
                                                                        \
    switch(cmd)                                                         \
    {                                                                   \
    case TREE_TYPENAME:                                                 \
        return (tree_p) #number;                                        \
                                                                        \
    case TREE_SIZE:                                                     \
        return (tree_p) sizeof(number##_t);                             \
                                                                        \
    case TREE_ARITY:                                                    \
        return (tree_p) 0;                                              \
                                                                        \
    case TREE_INITIALIZE:                                               \
        value = va_arg(va, va_type);                                    \
        number = (number##_p) malloc(sizeof(number##_t));               \
        number->value = value;                                          \
        return (tree_p) number;                                         \
                                                                        \
    case TREE_RENDER:                                                   \
        io = va_arg(va, tree_io_fn);                                    \
        stream = va_arg(va, void *);                                    \
                                                                        \
        number = (number##_p) tree;                                     \
        value = number->value;                                          \
        size = snprintf(buffer, sizeof(buffer), printf_format, value);  \
        if (io(stream, size, buffer) != size)                           \
            return NULL;                                                \
        return tree;                                                    \
                                                                        \
    default:                                                            \
        return tree_handler(cmd, tree, va);                             \
    }                                                                   \
}


#include "number.tbl"
