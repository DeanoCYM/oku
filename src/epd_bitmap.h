/* epd_bitmap.h
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

   Electronic paper bitmap buffer manipulation routines.

*/

#ifndef EPD_BITMAP_H
#define EPD_BITMAP_H

#include <sys/types.h>

/* Interface */

/* Allocate bitmap memory.

   Returns 0 Success.
           1 Failed to allocate bitmap memory, errno set to ENOMEM. */
int bitmap_create(void);

/* Allocate bitmap memory.

   Returns 0 Success.
           1 Failed to free bitmap memory, errno set to ECANCELED. */
int bitmap_destroy(void);

/* Modify specific pixels, if coordinates out of range, returns final
   pixel and sets errno.  */
int bitmap_px_toggle(uint16_t x, uint16_t y);
int bitmap_px_black(uint16_t x, uint16_t y);
int bitmap_px_white(uint16_t x, uint16_t y);

#endif	/* EPD_BITMAP_H */
