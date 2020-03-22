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

#include "bitmap.h"
#include "oku_types.h"
#include "oku_mem.h"

/* Determine minimum bytes required to hold pixel width resolution
   provided by W. */
#define PITCH(W) ((unsigned)(W % 8 ? (W / 8) + 1 : W / 8))

/************************/
/* Forward Declarations */
/************************/

/* Pixel operations */
static void px_toggle(byte *contains_px, byte bitmask);
static void px_unset(byte *contains_px, byte bitmask);
static void px_set(byte *contains_px, byte bitmask);

/* Dimensional analysis */
static resolution bitmap_height(BITMAP *bmp);
static size_t xy_to_index(members pitch, coordinate x, coordinate y);
static byte x_to_bitmask(coordinate x);

/* Validation */
static int check_coordinates(resolution width, resolution height,
			     coordinate x, coordinate y);
static int check_bitmap(BITMAP *bmp);
static int check_bitmaps_fit(BITMAP *bmp, BITMAP *rectangle,
			     coordinate x, coordinate y);
/************************/
/* Interface Definition */
/************************/

/* Function: bitmap_create()

   Allocates memory for bitmap objects. Determines required metrics of
   new bitmap object based on device dimensions. Results are stored in
   bmp. Returns handle to bitmap object, or NULL with invalid
   arguements. Exits on memory error.

   width  - pixel resolution (count).
   height - pixel resolution (count). */
BITMAP *
bitmap_create(resolution width, resolution height)
{
    if (width == 0 || height == 0)
	return NULL;

    BITMAP *bmp = oku_alloc(sizeof *bmp); /* exit on failure */

    /* When the pixel count is not a factor of 8, a partially filled
       byte with 'don't care' bits is required. */
    bmp->pitch  = PITCH(width);
    bmp->length = bmp->pitch * height;
    bmp->width  = width;

    /* Allocate memory for bitmap buffer (exits on failure). */
    bmp->buffer = oku_arrayalloc(bmp->length, sizeof bmp->length);

    return bmp;
}

/* Function: bitmap_assign()

   Populates an already allocated bitmap handle with the provided
   parameters.
*/
int
bitmap_ft(members length, members pitch, resolution width,
	      byte *buffer, BITMAP *out)
{
    if (buffer == NULL)
	return ERR_UNINITIALISED;

    /* Ensure that provided pitch is capable of holding the specified
       resolution across the width. */
    if ( PITCH(width) > pitch )
	return ERR_INPUT;

    out->buffer = buffer;
    out->length = length;
    out->pitch  = pitch;
    out->width  = width;

    return OK;
}

/* Function: bitmap_modify()

   Sets, unsets or toggles the colour of the pixels at the given
   coordinates according to the given mode.

   x,y - Cartesian coordinates of pixel.
   mode - Set to black, white or toggle pixel colour.
   pixel_colour - device logical representation of a black pixel

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL.
*/
int
bitmap_modify_px(BITMAP *bmp, coordinate x, coordinate y,
		 enum SET_PIXEL_MODE mode)
{
    int err = OK;

    /* Validate inputs */
    err = check_bitmap(bmp);
    if (err > 0) goto out;
    err = check_coordinates(bmp->width, bitmap_height(bmp), x, y);
    if (err > 0) goto out;

    /* Obtain the byte containing the bit and calculate the bit
       number */
    byte *contains_px = bmp->buffer + xy_to_index(bmp->pitch, x, y);
    byte bitmask = x_to_bitmask(x);

    switch (mode) {
    case SET_PIXEL_TOGGLE: px_toggle(contains_px, bitmask); break;
    case SET_PIXEL_BLACK:  px_set(contains_px, bitmask);  break;
    case SET_PIXEL_WHITE:  px_unset(contains_px, bitmask);    break;
    default: err = ERR_INPUT; /* should not reach */
    }

 out:	
    return err;
}

/* Function: bitmap_clear()

   Clear the bitmap by setting each pixel to white (0x00). */
int
bitmap_clear(BITMAP *bmp)
{
    int err = check_bitmap(bmp);
    if (err > 0) return err;

    for (members i = 0; i < bmp->length; ++i)
	bmp->buffer[i] = 0x00;

    return OK;
}

/* Function: bitmap_copy()

   Copy rectangle into bitmap buffer. It is likely that the two
   buffers are not byte aligned.

   The x and y coordinates are used to determine rectangle's target
   origin within bmp. The bits in input rectangle are then correctly
   aligned and copied into the bitmap in two operations as shown
   below.

    [1] Correct misalignment in input byte. Wipe bits to be replaced
	in output and combine with OR.

    [2] Retreive bits from input byte previously ignored. Wipe bits to
	be repaced in next output byte and combine with OR.

    [3] One full byte of input copied, increment appropriately.

    [4] preserve horizontal alignment, when the end of one row reached
	in rectangle, also move to new line in bitmap.

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int
bitmap_copy(BITMAP *bmp, BITMAP *rectangle, coordinate xmin, coordinate ymin)
{
    int err = OK;

    /* Validate inputs */
    err = check_bitmap(bmp) || check_bitmap(rectangle);
    if (err > 0) goto out;
    err = check_bitmaps_fit(bmp, rectangle, xmin, ymin);
    if (err > 0) goto out;

    /* Determine the start of input rectangle and its origin in
       output. */
    byte *in = rectangle->buffer;
    byte *out = bmp->buffer + xy_to_index(bmp->pitch, xmin, ymin);

    byte misalignment = xmin % 8;
    members written = 0;	          /* Bytes from input written to output */

    while ( written < rectangle->length ) {
	/* [1] - See function comments for process logic. */
	*out = (*in >> misalignment) |  (*out & ~(0xFF >> misalignment));
	++out;			             /* [2] */
	*out = (*in << (8 - misalignment)) | (*out & 0xFF >> misalignment);
	++in;			             /* [3] */
	++written;
	if (written % rectangle->pitch == 0) /* [4] */
	    out += bmp->pitch - rectangle->pitch;
    }

 out:
    return err;
}

/* Function: bitmap_destroy()

   Frees all memory associated with bitmap object. */
int
bitmap_destroy(BITMAP *bmp)
{
    if (bmp == NULL)
	goto fail;
    if (bmp->buffer == NULL)
	goto fail;

    oku_free(bmp->buffer);
    oku_free(bmp);

    return OK;
 fail:
    return ERR_UNINITIALISED;
}

/********************/
/* Static Functions */
/********************/

/* Static Function: bitmap_length()

   Returns the resolution of the bitmap in pixels.

   The height of the bitmap in pixels is the buffer length divided by
   the pitch, regardless of the presence of unused bits in the last
   byte of the pitch. This is also equal to the number of bytes store
   one column of data. */
static resolution
bitmap_height(BITMAP *bmp)
{
    return bmp->length / bmp->pitch;
}

/* Static Function: px_toggle

   Toggles bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_toggle(byte *contains_px, byte bitmask)
{
    *contains_px ^= bitmask;
    return;
}

/* Static Function: px_unset

   Unsets bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_unset(byte *contains_px, byte bitmask)
{
    *contains_px &= !bitmask;
    return;
}

/* Static Function: px_set

   Sets bits within provided byte according to mask.

   byte - 1B of data
   bit_mask - Bits to be modified represented by logical 1.
*/
static void
px_set(byte *contains_px, byte bitmask)
{
    *contains_px |= bitmask;
    return;
}

/* Static Function: xy_to_index

   Converts 2D coordinate into 1D index. Using the returned value will
   provide an index the byte containing the pixel at the provided
   coordintes. Note that the specific bit characterising the pixel
   remains undefined.

   pitch - Count of bytes in width.
   
   x, y - 2D coordinates. */
static members
xy_to_index(members pitch, coordinate x, coordinate y)
{
    return (y * pitch) + (x / 8);
}

/* Static Function: x_to_bitmask

   Returns a mask with the bit defining a pixel at the provided
   coordinates with x coordinate set to 1 and all other bits set to
   0. Mathematically, the calculation requires only the x
   coordinate. Most significant bit stores pixel data closest to
   origin. */
static byte
x_to_bitmask(coordinate x)
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
check_coordinates(resolution width, resolution height,
		  coordinate x, coordinate y)
{
    return (x >= width || y >= height) ? ERR_INPUT : OK;
}

/* Static function: check_bitmap()

   Check all numerical bitmap fields are non zero. */
static int
check_bitmap(BITMAP *bmp)
{
    return ( bmp->length == 0 || bmp->pitch == 0 || bmp->width == 0
	     || bmp->buffer == NULL )
	? ERR_UNINITIALISED : OK;
}

/* Static function: check_bitmaps_fit()

   Check that rectangle fits inside bmp considering the origin of
   rectangle is the coordinates (x,y) within bmp. */
static int
check_bitmaps_fit(BITMAP *bmp, BITMAP *rectangle,
		  coordinate x, coordinate y)
{
    return x + rectangle->width         > bmp->width
	|| y + bitmap_height(rectangle) > bitmap_height(bmp)
	? ERR_INPUT : OK;
}

