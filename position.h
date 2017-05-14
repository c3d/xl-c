#ifndef POSITION_H
#define POSITION_H
// ****************************************************************************
//  position.h                                      XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//     Record the positions in source files
//     Source code position is identified by a number that counts
//     characters parsed. Each time we open a new file, we identify
//     the current scanner position as the beginning of the file.
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

#include <stdbool.h>

typedef struct position
// ----------------------------------------------------------------------------
//    Identify a position within a file
// ----------------------------------------------------------------------------
{
    unsigned     position;      // Global position
    const char * file;          // File name
    unsigned     offset;        // Offset in file
    unsigned     line;          // Line number in file
    unsigned     column;        // Column in file
    unsigned     line_offset;   // Beginning of line
    unsigned     line_length;   // Length of source code
} position_t, *position_p;


// Getting and stepping the current global position
unsigned position();
unsigned position_step();

// Opening and closing source files
unsigned position_open_source_file(const char *name);

// Converting a global position into position information
bool     position_info(unsigned pos, position_p result);

// Getting the source code
bool     position_source(position_p posinfo, char *buffer, unsigned size);

#endif // POSITION_H
