// ****************************************************************************
//  position.c                                      XL - An extensible language
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
// ****************************************************************************
//  (C) 2017 Christophe de Dinechin <christophe@dinechin.org>
//   This software is licensed under the GNU General Public License v3
//   See LICENSE file for details.
// ****************************************************************************

#include "position.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



// ============================================================================
//
//    Positions lists
//
// ============================================================================

positions_p positions_new()
// ----------------------------------------------------------------------------
//    Allocating a new set of positions
// ----------------------------------------------------------------------------
{
    positions_p result = malloc(sizeof(position_t));
    result->position = 0;
    result->last = NULL;
    return result;
}


void positions_delete(positions_p p)
// ----------------------------------------------------------------------------
//   Delete positions records
// ----------------------------------------------------------------------------
{
    position_file_p f = p->last;
    while(f)
    {
        position_file_p prev = f->previous;
        free((void *) f->name);
        free(f);
        f = prev;
    }
}


srcpos_t position(positions_p p)
// ----------------------------------------------------------------------------
//    Return the current global position
// ----------------------------------------------------------------------------
{
    return p->position;
}


srcpos_t position_step(positions_p p)
// ----------------------------------------------------------------------------
//   Increment the current global position, return old location
// ----------------------------------------------------------------------------
{
    return p->position++;
}



// ============================================================================
//
//    Source files
//
// ============================================================================

srcpos_t position_open_source_file(positions_p p, const char *name)
// ----------------------------------------------------------------------------
//    Open a new source file
// ----------------------------------------------------------------------------
{
    position_file_p file = malloc(sizeof(position_file_t));
    file->name = strdup(name);
    file->start = position(p);
    file->previous = p->last;
    p->last = file;
    return file->start;
}


bool position_info(positions_p p, srcpos_t pos, position_p result)
// ----------------------------------------------------------------------------
// Converting a global position into position information
// ----------------------------------------------------------------------------
{
    if (!p)
        return false;

    position_file_p file = p->last;
    position_file_p good = NULL;

    while (file && file->start <= pos)
    {
        good = file;
        file = file->previous;
    }
    if (!good)
        return false;

    FILE *f = fopen(good->name, "rb");
    if (!f)
        return false;

    unsigned offset = pos - good->start;
    result->position = pos;
    result->file = file->name;
    result->offset = offset;

    unsigned line_offset = 0;
    unsigned current = 0;
    while (current < offset && !feof(f))
    {
        int c = fgetc(f);
        current++;
        if (c == '\n')
            line_offset = current;
    }
    result->line_offset = line_offset;
    result->column = offset - line_offset;
    while (!feof(f))
    {
        int c = fgetc(f);
        if (c == '\n')
            break;
        current++;
    }
    result->line_length = current - line_offset;
    fclose(f);
    return true;
}


bool position_source(position_p posinfo, char *buffer, unsigned size)
// ----------------------------------------------------------------------------
//   Read the source code based on the position information into target buffer
// ----------------------------------------------------------------------------
{
    FILE *f = fopen(posinfo->file, "rb");
    if (!f)
        return false;
    fseek(f, posinfo->line_offset, SEEK_SET);
    if (size > posinfo->line_length + 1)
        size = posinfo->line_length + 1;
    fread(buffer, 1, size-1, f);
    buffer[size-1] = 0;
    fclose(f);
    return true;
}
