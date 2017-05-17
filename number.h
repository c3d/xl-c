#ifndef NUMBER_H
#define NUMBER_H
// ****************************************************************************
//  number.h                                      XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of numbers in the XL compiler
//
//     Number nodes are used both to represent numbers in the source,
//     for example 245, as well as run-time numbers during execution.
//
//
//
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "tree.h"
#include <stdlib.h>
#include <stdint.h>


// Declaration of a number type
#define NUMBER(number, printf_format, reptype, vatype)                  \
typedef struct number                                                   \
{                                                                       \
    tree_t      tree;                                                   \
    reptype     value;                                                  \
} number##_t;                                                           \
tree_typedef(number);                                                   \
                                                                        \
                                                                        \
typedef struct based_##number                                           \
{                                                                       \
    number##_t  number;                                                 \
    unsigned    base;                                                   \
} based_##number##_t;                                                   \
tree_typedef(based_##number);                                           \
                                                                        \
                                                                        \
inline number##_r  number##_new(srcpos_t position, reptype value);      \
inline number##_r  based_##number##_new(srcpos_t position,              \
                                        reptype value, unsigned base);  \
inline reptype     number##_value(number##_p number);                   \
                                                                        \
inline number##_r  number##_make(tree_handler_fn, srcpos_t pos,         \
                                 reptype value, unsigned base);         \
extern tree_p      number##_handler(tree_cmd_t, tree_p, va_list);       \
extern tree_p      based_##number##_handler(tree_cmd_t,tree_p,va_list); \
                                                                        \
inline number##_r number##_make(tree_handler_fn h, srcpos_t pos,        \
                                reptype value, unsigned base)           \
{                                                                       \
    return (number##_r) tree_make(h, pos, value, base);                 \
}                                                                       \
                                                                        \
inline number##_r number##_new(srcpos_t position, reptype value)        \
{                                                                       \
    return number##_make(number##_handler, position, value, 10);        \
}                                                                       \
                                                                        \
inline number##_r based_##number##_new(srcpos_t position,               \
                                       reptype value, unsigned base)    \
{                                                                       \
    return number##_make(based_##number##_handler,position,value,base); \
}                                                                       \
                                                                        \
inline reptype number##_value(number##_p number)                        \
{                                                                       \
    return number->value;                                               \
}                                                                       \
                                                                        \
inline reptype based_##number##_value(based_##number##_p number)        \
{                                                                       \
    return number->number.value;                                        \
}


#include "number.tbl"

#endif // NUMBER_H
