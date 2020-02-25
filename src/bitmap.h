/* bitmap.h
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

/* Electronic paper display bitmap buffer control. The buffer is
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

#ifndef BITMAP_H
#define BITMAP_H

#include "oku_types.h"

/***********/
/* Objects */
/***********/

/* Object: BITMAP

   Structure containing bitmap dimensions. Also provides an pointer to
   a buffer which must be assigned manually.
 */
typedef struct BITMAP {
    byte *buffer;		/* Pointer to bitmap buffer */
    members length;		/* Length of buffer (1D) in bytes */
    members pitch;		/* Number of bytes in the width */
    resolution width;		/* Pixel count in one row */
} BITMAP;

enum SET_PIXEL_MODE { SET_PIXEL_BLACK, SET_PIXEL_WHITE, SET_PIXEL_TOGGLE };

/**************************/
/* Interface Deceleration */
/**************************/

/* Function: bitmap_create()

   Initialise bitmap object using electronic paper device
   dimensions. This does function does not allocate memory for the
   bitmap buffer in bmp->buffer. This must be done manually by
   allocating bmp->length bytes. */
BITMAP *bitmap_create(resolution width, resolution height);

/* Function: bitmap_modify()

   Sets, unsets or toggles the colour of the pixels at the given
   coordinates according to the given mode. */
int bitmap_modify_px(BITMAP *bmp, coordinate x, coordinate y,
		     enum SET_PIXEL_MODE mode);

/* Function: bitmap_clear()

   Clear the bitmap by setting each pixel to white. */
int bitmap_clear(BITMAP *bmp);

/* Function: bitmap_copy()

   Copy rectangle into bitmap buffer. */
int bitmap_copy(BITMAP *bmp, BITMAP *rectangle,
		coordinate xmin, coordinate ymin);

/* Function: bitmap_destroy()

   Free all memory allocated for bitmap_destroy. */
int bitmap_destroy(BITMAP *bmp);

#endif	/* BITMAP_H */
