/* utf8.c
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

/* Description: */

#include <stdio.h>

#include "oku_types.h"

/*************/
/* Constants */
/*************/
#define CHAR_INVALID 0x0000FFFD /* Replaces a missing or unsupported
				   Unicode character. */
#define CHAR_MISSING 0x000025A1 /* Replaces an invalid or
				   unrecognizable character. Indicates
				   a Unicode error. */

/************************/
/* Forward Declarations */
/************************/

static int file_read_n_bytes(FILE *src, byte *dest, unsigned *n);
static int file_read_seq(FILE *src, byte *dest, unsigned *n);
static unsigned seq_nbytes(byte first);
static codepoint utf8tocp(byte *utf8, unsigned len);

/***********************/
/* Interface Functions */
/***********************/

/* Function: utf8_next()

   Determines the next codepoint in a UTF-8 file and stores the value
   in out

   On successful decode of UTF-8 sequence into a unicode codepoint,
   codepoint is stored in out and 0 is returned.

   If invalid character is detected -1 is returned and an appropriate
   placeholder character is stored in codepoint.

   If the file cannot be read, 1 is returned and codepoint is set the
   the placeholder character. */
int
utf8_nextchar(FILE *src, codepoint *out)
{
    /* Create a buffer for a utf-8 sequence (maximum four bytes) */
    byte utf8[4] = { 0x00, 0x00, 0x00, 0x00 };
    unsigned length = 0;
    
    if ( 0 > file_read_seq(src, utf8, &length) ) {
	log_err("Failed to read file");
	return 1;
    }
	
    *out = utf8tocp(utf8, length);

    if ( *out == CHAR_INVALID ) {
	log_warn("Invalid character U+%04lu.", codepoint));
	return -1;
    }

    return 0;
}

/********************/
/* Static Functions */
/********************/

/* Static Function: file_read_n_bytes()

   Reads n bytes from src and stored them in dest. Returns 0 if the
   expected number of bytes are read or otherwise returns 1. */
static int
file_read_n_bytes(FILE *src, int n, byte *dest)
{
    return fread(dest, sizeof *dest, n, src) == n ? 0 : 1
}

/* Function: seq_nbytes()

   Returns the number of bytes determined using the first byte of a
   UTF-8 sequence. If the sequence is not a vaild initial byte of a
   UTF-8 sequence, 0 is returned.

   The length of any UTF-8 can be determined from the five most
   significant bits of the first byte. As shown in the table below,
   where x's represent unicode codepoint data.

   Firstly, bits are shifted right by 3 to retain only the five
   pertinent bits. This leaves 32 possible permiatations, which have
   been precalculated and stored in a look up table (lut).

   If the 5 bits do not match any of the four permissable values, the
   value returned is 0. 

   length byte[0]  byte[1]  byte[2]  byte[3]
   1      0xxxxxxx
   2      110xxxxx 10xxxxxx
   3      1110xxxx 10xxxxxx 10xxxxxx
   4      11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

   */
static unsigned
seq_nbytes(byte first)
{
    const unsigned len_decode[32] =
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };

    return len_decode[first >> 3];
}

/* Function: file_read_seq()

   Reads the UTF-8 file at src and records a single byte sequence and
   length in dest and len buffers respectively.

   Returns 0 on succuss. Returns 1 if not a valid UTF-8 file or if
   there is a problem reading the file.
 */
static int
file_read_seq(FILE *src, byte *dest, unsigned *n)
{
    /* Read first byte, do not increment buffer as the first byte
       is required to determine the number of trailing bytes. */
    if ( 1 != fread(dest, sizeof *dest, 1, src) )
	return 1;

    /* Record length of the sequence in bytes. This count is one
       greater than the number of trailing bytes remaining to be
       read. */
    *n = utf8_nbytes(*dest);

    /* Read each trailing byte, remembering that the first byte has
       already been read from src and recorded in dest.  */
    return fread(dest, sizeof *(dest+1), *n - 1, src) != (*n - 1) ? 1 : 0;
}

/* Function: utf8tocp()

   Converts a sequence of UTF-8 bytes into a single codepoint which is
   returned. If a valid sequence is not provied, the invalid character
   codepoint is returned.

*/
static int
utf8tocp(byte *utf8, unsigned len)
{
    /* Maximum allowable unicode length is . */
    codepoint cp = 0x00000000;

    switch ( len ) {
    case 0: 			/* Invalid length */
	return CHAR_INVALID;
	/* Convert a number of bytes equal to the sequence length. */
    case 1:		
	cp |= *utf8 & 0x7F;
	break;
    case 2:
	cp |= (*utf8++ & 0x1F) << 6;
	cp |= (*utf8   & 0x3F);
	break;
    case 3:
	cp |= (*utf8++ & 0x0F) << 12;
	cp |= (*utf8++ & 0x3F) << 6;
	cp |= (*utf8   & 0x3F);
	break;
    case 4:
	cp |= (*utf8++ & 0x07) << 18;
	cp |= (*utf8++ & 0x3F) << 12;
	cp |= (*utf8++ & 0x3F) << 6;
	cp |= (*utf8   & 0x3F);
	break;
	/* Should never be reached. */
    default:
	return CHAR_INVALID;
    }

    return cp;
}
