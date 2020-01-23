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

#include <sys/types.h>
#include "epd.h"

/***********/
/* Objects */
/***********/

/* Object: BITMAP

   Structure containing bitmap dimensions. Also provides an pointer to
   a buffer which must be assigned manually.
 */
typedef struct BITMAP {
    uint8_t *buffer;		/* Pointer to bitmap buffer */
    size_t length;		/* Length of buffer (1D) in bytes */
    size_t pitch;		/* Number of bytes in the width */
    size_t row_px;		/* Pixel count in one row */
} BITMAP;

enum SET_PIXEL_MODE { SET_PIXEL_BLACK, SET_PIXEL_WHITE, SET_PIXEL_TOGGLE };

/**************************/
/* Interface Deceleration */
/**************************/

/* Function: bitmap_create

   Initialise bitmap object using electronic paper device
   dimensions. This does function does not allocate memory for the
   bitmap buffer in bmp->buffer. This must be done manually by
   allocating bmp->length bytes.

   bmp - Bitmap object.
   width - Pixel count in width.
   height - Pixel count in height.

   Returns:
   0 Success.
   1 Invalid width or height, errno set to EINVAL.
*/
int bitmap_create(BITMAP *bmp, size_t width, size_t height);

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
int bitmap_modify_px(BITMAP *bmp, uint16_t x, uint16_t y,
		     enum SET_PIXEL_MODE mode, int pixel_colour);

/* Function: bitmap_clear

   Clear the bitmap by setting each pixel to white.

   bmp - Bitmap object (bmp->buffer must be allocated)
   black_colour - logical representation of a black pixel

   Returns:
   0 Success.
   1 Invalid black_colour, errno set to EINVAL.
   2 Invalid bitmap, errno set the ECANCELED.
 */
int bitmap_clear(BITMAP *bmp, int black_colour);
/* Function: bitmap_copy

   Copy rectangle into bitmap buffer.

   Returns:
   0 Success. 
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int bitmap_copy(BITMAP *bmp, BITMAP *rectangle, uint16_t xmin, uint16_t ymin);

#endif	/* BITMAP_H */
