#ifndef UTF8_H
#define UTF8_H
// ****************************************************************************
//  utf8.h                                          XL - An extensible language
// ****************************************************************************
//
//   File Description:
//
//      Simple utilities to deal with UTF-8 encoding
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

#include <ctype.h>

inline bool utf8_is_first(int c)
// ----------------------------------------------------------------------------
//   Recognize if a given code represents the first of a UTF-8 sequence
// ----------------------------------------------------------------------------
{
    return c >= 0xC0 && c <= 0xFD;
}

inline bool utf8_is_next(int c)
// ----------------------------------------------------------------------------
//   Recognize if a given code represents a valid 'next' in a UTF-8 sequence
// ----------------------------------------------------------------------------
{
    return c >= 0x80 && c <= 0xBF;
}

inline bool utf8_isalpha(int c)
// ----------------------------------------------------------------------------
//   Return true if character is alphanumeric or part of a UTF-8 sequence
// ----------------------------------------------------------------------------
{
    return isalpha(c) || utf8_is_first(c) || utf8_is_next(c);
}


inline unsigned utf8_previous(const char *text, unsigned position)
// ----------------------------------------------------------------------------
//   Finds the previous position in the text, assumed to be UTF-8
// ----------------------------------------------------------------------------
{
    if (position > 0)
    {
        position--;
        while (position > 0 && utf8_is_next(text[position]))
            position--;
    }
    return position;
}


inline unsigned utf8_next(const char *text, unsigned position)
// ----------------------------------------------------------------------------
//   Find the next position in the text, assumed to be UTF-8
// ----------------------------------------------------------------------------
{
    if (text[position])
    {
        position++;
        while (utf8_is_next(text[position]))
            position++;
    }
    return position;
}


inline unsigned utf8_code(const char *text)
// ----------------------------------------------------------------------------
//   Return the Unicode value for the character at the given position
// ----------------------------------------------------------------------------
{
    unsigned code = text[0];
    if (code & 0x80)
    {
        // Reference: Wikipedia UTF-8 description
        unsigned c1 = text[1];
        if (utf8_is_next(c1))
        {
            if ((code & 0xE0) == 0xC0)
                return ((code & 0x1F) << 6)
                    |   (c1   & 0x3F);

            unsigned c2 = text[2];
            if (utf8_is_next(c2))
            {
                if ((code & 0xF0) == 0xE0)
                    return ((code & 0xF)   << 12)
                        |  ((c1   & 0x3F)  << 6)
                        |   (c2   & 0x3F);
                unsigned c3 = text[3];
                if (utf8_is_next(c3))
                {
                    if ((code & 0xF8) == 0xF0)
                        return ((code & 0xF)  << 18)
                            |  ((c1   & 0x3F) << 12)
                            |  ((c2   & 0x3F) << 6)
                            |   (c3   & 0x3F);
                }
            }
        }
    }
    return code;
}


inline unsigned utf8_length(const char *text)
// ----------------------------------------------------------------------------
//    Return the length of the text in characters (not bytes)
// ----------------------------------------------------------------------------
{
    unsigned result = 0;
    for (int c = *text++; c; c = *text++)
        result += !utf8_is_next(c);
    return result;
}

#endif // UTF8_H
