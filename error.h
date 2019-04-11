#ifndef ERROR_H
#define ERROR_H
// ****************************************************************************
//  error.h                                         XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Error management for the XL compiler
//
//
//
//
//
//
//
//
// ****************************************************************************
//  (C) 1992-2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include <stdarg.h>
#include <stdlib.h>
#include <stdint.h>

typedef uintptr_t         srcpos_t;
typedef struct positions *positions_p;
typedef struct renderer  *renderer_p;
typedef struct errors    *errors_p;

// Reporting errors - This accepts an extended printf format with %t for trees
// Note that an implementation limitation requires trees to come first in args
extern void         error(srcpos_t position, const char *message, ...);
extern void         errorv(srcpos_t position, const char *message, va_list);
extern positions_p  error_positions(void);
extern positions_p  error_set_positions(positions_p);
extern renderer_p   error_renderer(void);
extern renderer_p   error_set_renderer(renderer_p);

// Providing an error context for complex, hierarchical errors
extern errors_p     errors_save(void);
extern void         errors_commit(errors_p errors);
extern void         errors_clear(errors_p errors);
extern unsigned     errors_count(void);

#endif // ERROR_H
