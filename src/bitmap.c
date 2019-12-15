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

/* Description:

   Electronic paper display bitmap buffer control. The buffer is
   designed to correspond to the portable bitmap format (PBM):

   Each row contains the same number of bits, packed 8 to a byte,
   don't care bits to fill out the last byte in the row if the width
   is not a factor of 8.

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
#include "epd.h"

struct Bitmap {
    uint8_t *bitmap;	/* Bitmap buffer */
    size_t length;	/* Length of bitmap buffer */
    size_t width;	/* Px count in one display row */
    size_t height;	/* Px count in one display column */
};

static struct Bitmap *raster = NULL;

/*** Forward Declarations ***/
static size_t bytes_in_width(void);
static size_t bytes_in_bitmap(void);
static size_t d2_to_d1(uint16_t x, uint16_t y);
static uint8_t* byte_at_index(uint16_t x, uint16_t y);
static uint8_t bit_number(uint16_t x);
static int check_bitmap(void);
static int check_d2_coordinates(uint16_t x, uint16_t y);

/*** Interface ***/

/* Allocates memory for bitmap buffer and container structure. Records
   the buffer length and initialises entire bitmap to 0.

   Returns:
   0 Success.
   1 Failed, memory error, sets errno to ENOMEM. */
int
bitmap_create(void)
{
    log_info("Creating new bitmap buffer");

    /* Allocate memory for container */
    raster = malloc(sizeof(*raster));
    if (raster == NULL)
	goto fail1;

    /* Import device dimensions */
    raster->width = epd_get_width();
    raster->height = epd_get_height();

    /* Length of structre is hardware specific and determined using
       device specific implementation of epd.h */
    raster->length = bytes_in_bitmap();
    log_info("Buffer (%dW x %dH) == %dB",
	     raster->width, raster->height, raster->length);

    /* Allocate memory for bitmap */
    raster->bitmap = calloc(raster->length, sizeof (uint8_t));
    if (raster->bitmap == NULL)
	goto fail2;

    return 0;
 fail2:
    raster->length = 0;
    free(raster);
    raster = NULL;
 fail1:
    log_err("Memory error on allocate");
    errno = ENOMEM;
    return 1;
}

/* Free memory allocated for bitmap.

   Returns:
   0 Success.
   1 Error, invalid pointer, errno set to ECANCELED*/
int
bitmap_destroy(void)
{
    log_info("Destroying bitmap buffer.");

    if (raster) {		/* Ensure raster is allocated */

	raster->width  = 0;
	raster->height = 0;
	raster->length = 0;

	if (raster->bitmap) {	/* Ensure bitmap allocated */
	    free(raster->bitmap);
	    raster->bitmap = NULL;
	} else {
	    log_warn("Attempted to free NULL raster");
	}

	free(raster);
	raster = NULL;

    } else {
	log_warn("Attempted to free NULL pointer.");
	errno = ECANCELED;
	return 1;
    }

    return 0;
}

/* Returns:
   Pointer to first element in bitmap or NULL on error */
uint8_t *
bitmap_get_raster(void)
{
    if(check_bitmap())
	return NULL;    
    return raster->bitmap;
}

size_t
bitmap_get_size(void)
{
    return raster->length;
}

/* Toggles logical state of bit defining pixel (in bitmap) at given
   coordinates.

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int
bitmap_px_toggle(uint16_t x, uint16_t y)
{
    if (check_bitmap())
	return 1;
    if (check_d2_coordinates(x, y))
	return 2;
    
    uint8_t *byte = byte_at_index(x, y);
    uint8_t bit_mask = bit_number(x);

    /* Toggle bit using mask, retaining all other bits. */
    *byte ^= bit_mask;

    return 0;
}

/* Unsets bit defining pixel (in bitmap) at given coordinates 

   Reterns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int
bitmap_px_black(uint16_t x, uint16_t y)
{
    if (check_bitmap())
	return 1;
    if (check_d2_coordinates(x, y))
	return 2;

    uint8_t *byte = byte_at_index(x, y);
    uint8_t bit_mask = bit_number(x);

    /* Black is represented with logical 0: unset bit using mask,
       retaining all other bits */
    *byte &= !bit_mask;
    
    return 0;
}

/* Sets bit defining pixel (in bitmap) at given coordinates.

   Returns:
   0 Success.
   1 At least one coordinate out of range, errno set to ECANCELED.
   2 Critical bitmap buffer error, errno set to*/
int
bitmap_px_white(uint16_t x, uint16_t y)
{
    if (check_bitmap())
	return 1;
    if (check_d2_coordinates(x, y))
	return 2;
    
    uint8_t *byte = byte_at_index(x, y);
    uint8_t bit_mask = bit_number(x);

    /* White is represented with logical 1: set bit using mask,
       retaining all other bits */
    *byte |= bit_mask;

    return 0;
}

/*** Static Functions ***/

/* Calculates number of bytes required to define one row of pixels
   across the width.

   Returns:
   Number of bytes defining pixels in width. */
static size_t
bytes_in_width(void)
{
    /* 1 byte can represent 8px across the width. However, an extra
       byte is required on each row if the number of px across the
       width is not a factor of 8.  */
    return (raster->width % 8 == 0)
	? raster->width / 8
	: raster->width / 8 + 1;
}

/* Calculates number of bytes required to define all pixels in display
   area.

   Returns: Bytes required buffer full bitmap. */
static size_t
bytes_in_bitmap(void)
{
    /* Each pixel of height requires the memory of one span of the
       width */
    return bytes_in_width() * raster->height;
}

/* Converts 2D array indicies into 1D index.

   Returns: 1D array index. */
static size_t
d2_to_d1(uint16_t x, uint16_t y)
{
    return (y * bytes_in_width()) + (x / 8);
}

/* Calculates the address of the byte containing the bit defining the
   pixel at given coordinate.

   Returns:
   Byte address containing pixel (x,y) */
static uint8_t*
byte_at_index(uint16_t x, uint16_t y)
{
    return raster->bitmap + d2_to_d1(x, y);
}

/* Returns a mask with the bit defining a pixel with x coordinate
   set to 1 and all other bits set to 0.

   Returns: Bit mask for pixel at given x coordinate. */
static uint8_t
bit_number(uint16_t x) { 

    /* Most significant bit stores pixel data closest to origin */
    return 0x01 << ( 7 - (x % 8) ); 
}

/* Ensures that bitmap is allocated and non-zero length.

   Returns:
   0 Vaild bitmap
   1 Bitmap unallocated or zero length, errno set to ECANCELED. */
static int
check_bitmap(void)
{
    if (!raster || !raster->bitmap || raster->length == 0) {
	log_err("Critical Error: Invalid bitmap.");
	errno = ECANCELED;
	return 1;
    }
    
    return 0;
}

/* Ensure that the x and y coordiantes provided are within the device
   dimensions.

   Returns:
   0 Valid coordinates
   1 Invalid coordinates, errno set to EINVAL. */
static int
check_d2_coordinates(uint16_t x, uint16_t y)
{
    /* Raster dimensions are counts, and so are one less than the
       maximum allowable coordinate. */
    if ( x >= raster->width || y >= raster->height ) {
	errno = ECANCELED;
	log_err("Coordinate (%d,%d) overflows buffer, max(%d,%d)",
		x, y, raster->width, raster->height);
	errno = EINVAL;	
	return 1;
    }
	
    return 0;
}
