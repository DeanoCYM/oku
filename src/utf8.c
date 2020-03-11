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

/***************/
/* Description */
/***************/
/* Decodes UTF-8 stream into unicode codepoint integers. */

#include <stdio.h> 		/* FILE* */
#include <errno.h>		/* errno */

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

static unsigned seq_nbytes(byte first);
static int file_read(FILE *src, byte *dest, unsigned n);
static int ftoutf8(FILE *src, byte *dest, unsigned *n);
static int utf8tocp(byte *utf8, unsigned len, codepoint *out);
static int check_eof(FILE* textfile);

/***********************/
/* Interface Functions */
/***********************/

/* Function: utf8_ftocp()

   Determines the next codepoint in a UTF-8 file and stores the value
   in *out.

   On successful decode of UTF-8 sequence into a unicode codepoint,
   codepoint is stored in *out.

   If invalid character is read, the appropriate placeholder codepoint
   is stored in *out and a warning code returned. */
int
utf8_ftocp(FILE *src, codepoint *out)
{
    int err = OK;

    /* Create a buffer for a utf-8 sequence (maximum four bytes) and
       an integer to store its length. */
    byte utf8[4] = { 0x00, 0x00, 0x00, 0x00 };
    unsigned length = 0;

    err = ftoutf8(src, utf8, &length);
    if (err > 0)
	goto out;
	
    err = utf8tocp(utf8, length, out);

 out:
    return err;
}

/********************/
/* Static Functions */
/********************/

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
    const unsigned utf8_len_decode[32] =
	{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	  0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0 };

    return utf8_len_decode[first >> 3];
}

/* Function: file_read()

   Reads n bytes from the file at src into the buffer dest and returns
   0 on success.

   If less than n bytes are read, the file is checked for EOF. If EOF
   has been reached the function returns WARN_EOF. If less bytes are
   read becuase of an issue with the stream ERR_IO is returned. */

static int
file_read(FILE *src, byte *dest, unsigned n)
{
    if (fread(dest, sizeof *dest, n, src) < n)
	return check_eof(src) || ERR_IO;

    return OK;
}

/* Function: ftoutf8()

   [1] Read first byte of a UTF-8 sequence into dest.

   [2] Use the first byte of the sequence to determine the number of
   octets in the sequence and store this value in n.

   [3] Read any remaining octets of the UTF-8 sequence and store in
   dest.

   Returns:

   OK: UTF-8 seqence was successfully read.
   WARN_EOF: Attemped read passed end of file.
   ERR_IO: Failed to read stream.
*/
static int
ftoutf8(FILE *src, byte *dest, unsigned *n)
{
    int err = OK;

    err = file_read(src, dest, 1); /* [1] */
    if (err)
	return err;

    *n = seq_nbytes(*dest);	   /* [2] */

    if (*n > 1 && *n <= 4)	   /* [3]  */
	err = file_read(src, dest, *n - 1);
    else if (*n != 1)		   /* Invalid UTF-8 */
	err = ERR_INVALID_UTF8;

    return err;
}

/* Function: utf8tocp()

   Decodes UTF-8 octet sequence into a single codepoint which is
   recorded as a codepoint in out.

   If a valid sequence is not provied, the codepoint is set to (the
   valid unicode) CHAR_INVALID and WARN_REPLACEMENT_CHAR is returned.

*/
static int
utf8tocp(byte *utf8, unsigned len, codepoint *out)
{
    int err = OK;

    switch ( len ) {
	/* Decode a number of bytes equal to the sequence length. */
    case 1:		
	*out |= *utf8 & 0x7F;
	break;
    case 2:
	*out |= (*utf8++ & 0x1F) << 6;
	*out |= (*utf8   & 0x3F);
	break;
    case 3:
	*out |= (*utf8++ & 0x0F) << 12;
	*out |= (*utf8++ & 0x3F) << 6;
	*out |= (*utf8   & 0x3F);
	break;
    case 4:
	*out |= (*utf8++ & 0x07) << 18;
	*out |= (*utf8++ & 0x3F) << 12;
	*out |= (*utf8++ & 0x3F) << 6;
	*out |= (*utf8   & 0x3F);
	break;
	/* Invalid UTF-8 (Fallthrough) */
    case 0:
    default:			
	*out = CHAR_INVALID;
	err = WARN_REPLACEMENT_CHAR;
    }

    return err;
}

/* Function: check_eof()

   Returns WARN_EOF if EOF is reached, OK (0) otherwise. Resets error
   number in case of EOF. */
static int
check_eof(FILE* textfile)
{
    return feof(textfile) ? errno = 0, WARN_EOF : OK;
}
