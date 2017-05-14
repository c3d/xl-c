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
} name_t, *name_p;


inline name_p      name_new(unsigned position, size_t sz, const char *data);
inline void        name_delete(name_p name);
inline name_p      name_append(name_p name, size_t sz, const char *data);
inline const char *name_data(name_p name);
inline size_t      name_size(name_p name);
extern bool        name_is_operator(name_p name);
extern bool        name_is_valid(size_t size, const char *data);

// Private name handler, should not be called directly in general
extern tree_p name_handler(tree_cmd_t cmd, tree_p tree, va_list va);

// Helper macro to initialize with a C constant
#define name_cnew(pos, name)    name_new(pos, sizeof(name), name)



// ============================================================================
//
//   Inline implementations
//
// ============================================================================

inline name_p name_new(unsigned position, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//    Allocate a name with the given data
// ----------------------------------------------------------------------------
{
    return (name_p) text_make(name_handler, position, sz, data);
}


inline void name_delete(name_p name)
// ----------------------------------------------------------------------------
//   Delete the given name
// ----------------------------------------------------------------------------
{
    tree_delete((tree_p) name);
}


inline name_p name_append(name_p name, size_t sz, const char *data)
// ----------------------------------------------------------------------------
//   Append name, possibly in place
// ----------------------------------------------------------------------------
{
    return (name_p) text_append((text_p) name, sz, data);
}


inline const char *name_data(name_p name)
// ----------------------------------------------------------------------------
//   Return the data for the name
// ----------------------------------------------------------------------------
{
    return text_data((text_p) name);
}


inline size_t name_size(name_p name)
// ----------------------------------------------------------------------------
//   Return the data for the name
// ----------------------------------------------------------------------------
{
    return text_size((text_p) name);
}

#endif // NAME_H
