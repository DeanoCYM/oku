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

   Electronic paper display bitmap buffer control.

*/

#include <sys/types.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <ert_log.h>

#include "bitmap.h"
#include "epd.h"


static uint8_t *bitmap = NULL;	/* Bitmap buffer */

/*** Forward Declarations ***/
static size_t display_area(void);
static int alloc_bitmap(void);
static int free_bitmap(void);
static uint8_t* byte_offset(uint16_t x, uint16_t y);
static uint8_t bit_number(uint16_t x);
static int invalid_bitmap(void);

/*** Interface ***/

/* Allocate memory for bitmap.

   Returns: 0 Success.
            1 Failed, memory error, sets errno to ENOMEM. */
int
bitmap_create(void)
{
    log_info("Creating %zuB buffer for %zuW %zuH bitmap",
	     display_area(), epd_get_width(), epd_get_height());

    return alloc_bitmap();
}

/* Free memory allocated for bitmap.

   Returns: 0 Success.
            1 Error, invalid pointer, errno set to ECANCELED*/
int
bitmap_destroy(void)
{
    log_info("Freeing bitmap buffer. ");
    return free_bitmap();
}

/* Toggles logical state of bit defining pixel (in bitmap) at given
   coordinates. */
int
bitmap_px_toggle(uint16_t x, uint16_t y)
{
    if (invalid_bitmap())
	return 1;

    uint8_t *byte = byte_offset(x, y);
    uint8_t bitnumber_mask = bit_number(x);

    /* Toggle bit using mask, retaining all other bits. */
    *byte ^= bitnumber_mask;

    return 0;
}

/* Unsets bit defining pixel (in bitmap) at given coordinates */
int
bitmap_px_black(uint16_t x, uint16_t y)
{
    if (invalid_bitmap())
	return 1;

    uint8_t *byte = byte_offset(x, y);
    uint8_t bitnumber_mask = bit_number(x);

    /* Black is represented with logical 0: unset bit using mask,
       retaining all other bits */
    *byte &= !bitnumber_mask;
    
    return 0;
}

/* Sets bit defining pixel (in bitmap) at given coordinates */
int
bitmap_px_white(uint16_t x, uint16_t y)
{
    if (invalid_bitmap())
	return 1;

    uint8_t *byte = byte_offset(x, y);
    uint8_t bitnumber_mask = bit_number(x);

    /* White is represented with logical 1: set bit using mask,
       retaining all other bits */
    *byte |= bitnumber_mask;

    return 0;
}

/*** Static Functions ***/

/* Calculates number of bytes required to define the entire epaper
   display area.

   Returns: Bytes required buffer full bitmap. */
static size_t
display_area(void)
{
    /* 1 byte can represent 8px across the width. However, an extra
       byte is required on each row if the number of px across the
       width is not a factor of 8.  */

    const size_t width = (epd_get_width() % 8 == 0)
	? epd_get_width() / 8
	: epd_get_width() / 8 + 1;
    
    return width * epd_get_height();
}


/* Allocates memory for bitmap buffer (file scope).

   Returns: 0  Success
            1  Memory allocation error, errno set to ENOMEM */
static int
alloc_bitmap(void)
{
    bitmap = calloc(display_area(), sizeof (uint8_t));
    if (!bitmap) {
	log_err("Failed to allocate bitmap buffer");
	errno = ENOMEM;
	return 1;
    }

    return 0;
}

/* Free dynamically allocated bitmap buffer.

   Returns: 0 Success.
            1 Fail, errno set to ECANCELED */
static int
free_bitmap(void)
{
    if (invalid_bitmap()) {
	log_warn("Cannot free buffer, bitmap not allocated");
	errno = ECANCELED;
	return 1;
    }

    free(bitmap);
    bitmap = NULL;

    return 0;
}

/* Returns a pointer to bitmap, offset to the byte that contains the
   bit defining the pixel at the given coordinates. */
static uint8_t*
byte_offset(uint16_t x, uint16_t y)
{
    /* Eight pixels in x axis is one byte. One pixel in y axis is one
       byte. */    
    size_t offset = ( (size_t)x * y ) + ( x / 8 );

    return &bitmap[offset];
}

/* Returns bit number defining the pixel state at the given x
   coordinate. */
static uint8_t
bit_number(uint16_t x)
{
    /* Binary and pixel representation is opposite, i.e origin pixel
       data is in the right most bit of first byte (0000 0001) and so
       should not be shifted. */
    return 0x01 << (x % 8);
}

/* Returns 1 and sets errno to ECANCELED if bitmap is an invalid
   pointer. Returns 0 otherwise. */
static int
invalid_bitmap(void)
{
    if (NULL == bitmap) {
	errno = ECANCELED;
	log_warn("Attempted to free invalid pointer.");
	errno = ECANCELED;
    return 1;
    }

    return 0;
}
