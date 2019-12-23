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

#ifndef BITMAP_H
#define BITMAP_H

#include <sys/types.h>

/* Convert between one and two dimensional indicies  */
#define D2_TO_D1(X, Y) ((X) + ( (Y) * bitmap_get_pitch() ))
#define D1_TO_X(LEN)   ( (LEN) - ( bitmap_get_rows() * bitmap_get_pitch() )
#define D1_TO_Y(LEN)   ( ((LEN) - D1_TO_X(LEN)) / bitmap_get_pitch() )

/* Interface */

/* Allocate bitmap memory.

   Returns:
   Pointer to Bitmap data structure.
   NULL, Failed to allocate bitmap memory, errno set to ENOMEM. */
int bitmap_create(void);

/* Free bitmap memory.

   Returns:
   0 Success.
   1 Failed to free bitmap memory, errno set to ECANCELED. */
int bitmap_destroy(void);

/* Modify specific pixels.

   Returns:
   0 Success.
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int bitmap_px_toggle(uint16_t x, uint16_t y);
int bitmap_px_black(uint16_t x, uint16_t y);
int bitmap_px_white(uint16_t x, uint16_t y);

/* Copy rectangle into bitmap buffer

   Returns:
   0 Success. 
   1 Critical bitmap buffer error, errno set to ECANCELED.
   2 At least one coordinate out of range, errno set to EINVAL. */
int bitmap_copy(uint8_t *bitmap, uint16_t x, uint16_t y);

/* Returns:
   Pointer to start of bitmap */
uint8_t *bitmap_get_raster(void);

/* Returns:
   Length of bitmap in bytes */
size_t bitmap_get_size(void);
size_t bitmap_get_pitch(void);
size_t bitmap_get_rows(void);

#endif	/* BITMAP_H */
