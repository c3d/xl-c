#ifndef NAME_H
#define NAME_H
// ****************************************************************************
//  name.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Representation of names in the XL compiler
//     Name nodes are used to represent names like ABC and symbols like +=
//     It is extremely similar to text internally
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

typedef struct name
// ----------------------------------------------------------------------------
//   A name internal representation is little more than a stream of bytes
// ----------------------------------------------------------------------------
//   The bytes are allocated immediately after the name_t structure
{
    text_t      text;           // The base blob
} name_t;

#ifdef NAME_C
#define inline extern inline
#endif

blob_type(char, name);
extern bool name_is_operator(name_p name);
extern bool name_is_valid(size_t size, const char *data);
inline bool name_eq(text_p, const char *value);

// Private name handler, should not be called directly in general
extern tree_p   name_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Helper macro to initialize with a C constant
#define name_cnew(pos, name)    name_new(pos, sizeof(name), name)

#undef inline


// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline bool name_eq(text_p name, const char *str)
// ----------------------------------------------------------------------------
//   Check if the text matches some string constant
// ----------------------------------------------------------------------------
{
    return text_eq((text_p) name, str);
}


#endif // NAME_H
