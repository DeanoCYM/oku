/* bitmap.c
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
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
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

/*  Electronic paper display bitmap buffer control. The buffer is
   designed to correspond to the portable bitmap format (PBM):

   Each row contains the same number of bits, packed 8 to a byte,
   don't care bits to fill out the last byte in the row if the width
   is not a factor of 8. The pitch is the number of bytes in one row.

   Each bit represents a pixel: 1 is black, 0 is white.

   The order of the pixels is left to right. The order of their
   storage within each file byte is most significant bit to least
   significant bit. The order of the file bytes is from the beginning
   of the file toward the end of the file.

*/

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <ert_log.h>

#include "bitmap.h"

/************************/
/* Forward Declarations */
/************************/

static void px_toggle(uint8_t *byte, uint8_t bit_mask);
static void px_unset(uint8_t *byte, uint8_t bit_mask);
static void px_set(uint8_t *byte, uint8_t bit_mask);
static size_t xy_to_index(size_t pitch, uint16_t x, uint16_t y);
static uint8_t x_to_bitmask(uint16_t x);
static int check_coordinates(size_t width, size_t height,
			     uint16_t x, uint16_t y);
static int check_bitmap(BITMAP *bmp);

/************************/
/* Interface Definition */
/************************/

/* Function: bitmap_create

   Determines required metrics of new bitmap object based on device
   dimensions. Results are stored in bmp.

   bmp - Bitmap object.
   width - Pixel count in width.
   height - Pixel count in height.

   Returns:
   0 Success.
   1 Invalid width or height, errno set to EINVAL.
*/
int
bitmap_create(BITMAP *bmp, size_t width, size_t height)
{
    if (width == 0 || height == 0) {
	log_err("Invalid bitmap dimensions %zuW%zuH px.",
		width, height);
	errno = EINVAL;
	return 1;
    }

    /* When the pixel count is not a factor of 8, a partially filled
       byte with 'don't care' bits is required. */
    bmp->pitch  = width % 8 ? (width / 8) + 1 : width / 8;
    bmp->length = bmp->pitch * height;
    bmp->row_px = width;
    bmp->buffer = NULL;		/* must be manually assigned */

    log_info("For %zuW x %zuH px device,\n"
	     "New %zuW x %zuH px bitmap created (Total %zuB),\n"
	     "%zuBit(s) unused in each %zuB row.  ",
	     width, height,
	     bmp->pitch * 8, bmp->length / bmp->pitch, bmp->length,
	     bmp->row_px % 8, bmp->pitch);

    return 0;
}

/* Function: bitmap_modify

   Sets, unsets or toggles the colour of the pixels at the given
   coordinates according to the given mode.

   bmp - Bitmap object (bmp->buffer must be allocated).
   x,y - Cartesian coordinates of pixel.
   mode - Set to black, white or toggle pixel colour.
   pixel_colour - device logical representation of a black pixel

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL.
*/
int
bitmap_modify_px(BITMAP *bmp, uint16_t x, uint16_t y,
		 enum SET_PIXEL_MODE mode, int pixel_colour)
{
    if ( check_bitmap(bmp) ) {
	log_err("Invalid bitmap.");
	errno = ECANCELED;
	return 1;
    }

    /*  Length of the buffer divided by the pitch gives height of the
	display in pixels */
    if ( check_coordinates(bmp->row_px, bmp->length / bmp->pitch, x, y) ) {
	log_err("Invalid coordinates");
	errno = EINVAL;
	return 2;
    }
    
    /* Obtain the byte containing the bit and calculated the bit
       number */
    uint8_t *byte = bmp->buffer + xy_to_index(bmp->pitch, x, y);
    uint8_t bitmask = x_to_bitmask(x);

    /* If the device does not conform to the convention of a black
       pixel being represented by a logical 1, then the operating
       modes must be reversed. This does not effect toggle mode which
       always inverts the bit.  */
    if (pixel_colour == 0 || mode == SET_PIXEL_BLACK)
	mode = SET_PIXEL_WHITE;
    else if (pixel_colour == 0 || mode == SET_PIXEL_WHITE)
	mode = SET_PIXEL_BLACK;
    
    switch (mode) {
    case SET_PIXEL_TOGGLE: px_toggle(byte, bitmask); break;
    case SET_PIXEL_BLACK:  px_set(byte, bitmask);    break;
    case SET_PIXEL_WHITE:  px_unset(byte, bitmask);  break;
    }
	
    return 0;
}

/* Function: bitmap_clear

   Clear the bitmap by setting each pixel to white.

   bmp - Bitmap object (bmp->buffer must be allocated)
   black_colour - logical representation of a black pixel

   Returns:
   0 Success.
   1 Invalid black_colour, errno set to EINVAL.
   2 Invalid bitmap, errno set the ECANCELED.
 */
int
bitmap_clear(BITMAP *bmp, int black_colour)
{
    if (black_colour != 0 && black_colour != 1) {
	log_err("Invalid black representation.");
	errno = EINVAL;
	return 1;
    }

    if ( check_bitmap(bmp) ) {
	log_err("Invalid bitmap.");
	errno = ECANCELED;
	return 2;
    }

    /* If black is represented by zero, each pixel in byte to white
       0xFF. Do inverse if black represented by 1. */
    for (unsigned int i = 0; i < bmp->length; ++i) {
	bmp->buffer[i] = black_colour ? 0x00 : 0xFF;
    }

    log_info("Bitmap cleared.");

    return 0;
}

/* Function: bitmap_copy

   Copy rectangle into bitmap buffer. It is likely that the two
   buffers are not byte aligned.

   The x and y coordinates are used to determine rectangle's target
   origin within bmp. The x coordinate is used to determine the bit
   number of the starting point in the output buffer.

   Mask only the required bits from the input buffer.

   e.g. let x == 14
   x % 8 = 6, 0xFF << 6 = 1100 0000

   The mask is used to remove the unneeded bits from the end of the
   input byte and store the results in a temporary byte.

   mask &      byte -> tmp
   1110 0000 & 1010 1010 -> 1010 0000

   The data in the tmp byte needs to be shifted to the right to
   reflect its intended position in the output byte.

   tmp >> (x % 8) -> tmp
   1010 0000 >> (5 % 8) -> 0000 0101

   The data is then copied to the output byte again using the bitmask
   to avoid data loss from previous bits.

   A similar proceedure is used to mask and copy the remaining
   uncopied data in the input byte to the next output byte.

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int
bitmap_copy(BITMAP *bmp, BITMAP *rectangle, uint16_t xmin, uint16_t ymin)
{
    /* Retrieve the start of input rectangle and the target origin in
       output */
    uint8_t *in = rectangle->buffer;
    uint8_t *out = bmp->buffer + xy_to_index(bmp->pitch, xmin, ymin);

    size_t count;
    uint8_t in_shifted, out_masked, mask;
    while ( count < rectangle->length ) {

	mask = 0xFF << ( xmin % 8 );
	in_shifted = ( mask & *in ) >> ( xmin % 8 );
    
	/* Clear bits to be overwritten in out using mask. Write the
	   data from the temporary byte. */
	out_masked = *out & mask;
	*out = out_masked & in_shifted;
	++out;			/* Current output byte complete  */

	/* The data from input byte masked off previously needs to be
	   copied to the start of the next byte of the output. */
	in_shifted = ( ~mask & *in ) << ( 8 - ( xmin % 8 ) );
	out_masked = *out & (~mask);
	*out = out_masked & in_shifted;
	++in;			/* Current input byte complete */

	/* Increment bytes written counter */
	++count;

	/* Increment one line in the output buffer if the end of input
	   buffer row */
	if ( count % rectangle->pitch == 0 )
	    out += bmp->pitch;
    }
    
    return 0;
}
/********************/
/* Static Functions */
/********************/

/* Static Function: px_toggle

   Toggles bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_toggle(uint8_t *byte, uint8_t bit_mask)
{
    *byte ^= bit_mask;
    return;
}

/* Static Function: px_unset

   Unsets bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_unset(uint8_t *byte, uint8_t bit_mask)
{
    *byte &= !bit_mask;
    return;
}

/* Static Function: px_set

   Sets bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_set(uint8_t *byte, uint8_t bit_mask)
{
    *byte |= bit_mask;
    return;
}

/* Static Function: xy_to_index

   Converts 2D coordinate into 1D index. Using the returned value will
   provide an index the byte containing the pixel at the provided
   coordintes. Note that the specific bit characterising the pixel
   remains undefined.

   pitch - Count of bytes in width.
   
   x, y - 2D Cartesian coordinates.

   Returns: Array index of byte containing pixel.
*/
static size_t
xy_to_index(size_t pitch, uint16_t x, uint16_t y)
{
    return (y * pitch) + (x / 8);
}

/* Static Function: x_to_bitmask

   Returns a mask with the bit defining a pixel at the provided
   coordinates with x coordinate set to 1 and all other bits set to
   0. Mathematically, bit number requires only the x coordinate. Most
   significant bit stores pixel data closest to origin.

   x - Cartesian coordinate

   Returns: Bit mask for pixel at given x coordinate.
*/
static uint8_t
x_to_bitmask(uint16_t x)
{ 
    return 0x01 << ( 7 - (x % 8) ); 
}

/* Static function: check_coordinates

   Ensure that the x and y coordiantes provided are within the bitmap
   dimensions. Bitmap width and height are counts, and so must be at
   least one less than the maximum allowable x and y coordinates
   respectively.

   width - Count of pixels in row
   height - Count of pixels in height
   x,y - pixel coordinates starting from origin (0,0)

   Returns:
   0 Coordinates within valid range.
   1 Invalid coordinates */
static int
check_coordinates(size_t width, size_t height, uint16_t x, uint16_t y)
{
    return (x >= width || y >= height) ? 1 : 0;
}

/* Static function: check_bitmap

   Check all numerical bitmap fields are non zero.

   bmp - Bitmap object.

   Returns:
   0 Valid dimensions.
   1 Invalid dimensions.
*/
static int
check_bitmap(BITMAP *bmp)
{
    return ( bmp->length == 0 || bmp->pitch == 0 || bmp->row_px == 0 )
	? 1 : 0;
}
