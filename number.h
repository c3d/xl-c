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
} number##_t, *number##_p;                                              \
                                                                        \
                                                                        \
typedef struct based_##number                                           \
{                                                                       \
    number##_t  number;                                                 \
    unsigned    base;                                                   \
} based_##number##_t, *based_##number##_p;                              \
                                                                        \
                                                                        \
inline number##_p  number##_new(unsigned position, reptype value);      \
inline number##_p  based_##number##_new(unsigned position,              \
                                        reptype value, unsigned base);  \
inline void        number##_delete(number##_p number);                  \
inline reptype     number##_data(number##_p number);                    \
                                                                        \
inline number##_p  number##_make(tree_handler_fn, unsigned pos,         \
                                 reptype value, unsigned base);         \
extern tree_p      number##_handler(tree_cmd_t, tree_p, va_list);       \
extern tree_p      based_##number##_handler(tree_cmd_t,tree_p,va_list); \
                                                                        \
inline number##_p number##_make(tree_handler_fn h, unsigned pos,        \
                                reptype value, unsigned base)           \
{                                                                       \
    return (number##_p) tree_make(h, pos, value, base);                 \
}                                                                       \
                                                                        \
inline number##_p number##_new(unsigned position, reptype value)        \
{                                                                       \
    return number##_make(number##_handler, position, value, 10);        \
}                                                                       \
                                                                        \
inline number##_p based_##number##_new(unsigned position,               \
                                       reptype value, unsigned base)    \
{                                                                       \
    return number##_make(based_##number##_handler,position,value,base); \
}                                                                       \
                                                                        \
inline void number##_delete(number##_p number)                          \
{                                                                       \
    tree_delete((tree_p) number);                                       \
}                                                                       \
                                                                        \
inline reptype number##_data(number##_p number)                         \
{                                                                       \
    return number->value;                                               \
}


#include "number.tbl"

#endif // NUMBER_H
