/* oku_types.h
 * 
 * This file is part of oku.
 *
 * Copyright (C) 2019 Ellis Rhys Thomas
 * 
 * oku is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * oku is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.

 * You should have received a copy of the GNU General Public License
 * along with oku.  If not, see <https://www.gnu.org/licenses/>.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS OR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* Description:

   Defines aliases and describes oku data types and error return
   integers.
*/

#ifndef OKU_TYPES_H
#define OKU_TYPES_H

#include <stddef.h>		/* size_t */

/* Local coordinate system to describle recangles, based on a positive
   distance from an origin. The origin should be the upper left most
   corner of the rectangle so by definition the coordinate can never
   be negative.

   The coordinate system is used it identify specific pixels in a
   display device where (coordinate x == 0, coordinate y == 0)
   corresponds to the origin.  */
typedef unsigned short int coordinate;

/* A point count in one of the two planes of the coordinate
   system. Typically used to count a number of pixels across one of
   the axis. */
typedef unsigned short int resolution;

/* A single octet of data. */
typedef unsigned char byte;

/* Number of elements in an array */
typedef size_t members;

/* Unicode codepoint, a numerical value that describes a single
   glyph. */
typedef unsigned long int codepoint;

/* Error codes */
enum OKU_ERRNO
    {
     /* No errors */
     OK                            = 0x00,
     /* Errors (Positive) */
     ERR_INPUT                     = 0x01,
     ERR_COMMS                     = 0x02,
     ERR_MEM                       = 0x03,
     ERR_IO                        = 0x04,
     ERR_UNINITIALISED             = 0x05,
     ERR_PARTIAL_WRITE   	   = 0x06,
     ERR_BUSY                      = 0x07,
     ERR_INVALID_UTF8              = 0x08,
     ERR_NOT_FOUND                 = 0x08,
     ERR_RENDER                    = 0x09,
     ERR_INITIALISED               = 0x0A,
     /* Warnings (Negative) */
     WARN_ROOT                     = -0x01,
     WARN_REPLACEMENT_CHAR         = -0x02,
     WARN_MTBUFFER                 = -0x03,
     WARN_EOF                      = -0x04
    };

#endif	/* OKU_TYPES_H */
