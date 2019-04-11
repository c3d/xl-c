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
#include <stdint.h>
#include <stdlib.h>

typedef uintptr_t srcpos_t;


typedef struct position
// ----------------------------------------------------------------------------
//    Identify a position within a file
// ----------------------------------------------------------------------------
{
    srcpos_t     position;      // Global position
    const char * file;          // File name
    unsigned     offset;        // Offset in file
    unsigned     line;          // Line number in file
    unsigned     column;        // Column in file
    unsigned     line_offset;   // Beginning of line
    unsigned     line_length;   // Length of source code
} position_t, *position_p;


typedef struct position_file
// ----------------------------------------------------------------------------
//    List of input files
// ----------------------------------------------------------------------------
{
    const char *          name;
    srcpos_t              start;
    struct position_file *previous;
} position_file_t, *position_file_p;


typedef struct positions
// ----------------------------------------------------------------------------
//   Record positions
// ----------------------------------------------------------------------------
{
    srcpos_t         position;
    position_file_t *last;
} positions_t, *positions_p;


// Creating and deleting positions
positions_p positions_new();
void        positions_delete(positions_p p);

// Getting and stepping the current global position
srcpos_t position(positions_p p);
srcpos_t position_step(positions_p p);

// Opening and closing source files
srcpos_t position_open_source_file(positions_p p, const char *name);

// Converting a global position into position information
bool     position_info(positions_p p, srcpos_t pos, position_p result);

// Getting the source code
bool     position_source(position_p posinfo, char *buffer, size_t size);

#endif // POSITION_H
