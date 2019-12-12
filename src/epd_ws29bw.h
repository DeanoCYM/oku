/* epd_ws29bw.c
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
 *
 * Communication with display device. Requires hardware specific
 * implementation.
 * 
 */

#ifndef EPD_H
#define EPD_H

#include <sys/types.h>
#include <stdint.h>

/* Device logical representation of pixel colour. */
enum PIXEL_COLOUR { BLACK, WHITE };

/* Interface */

/* Fully initialises the device. Must be performed prior to any other
   routines and to wake from sleep.

   Returns: 0  Success.
            1  Fail, errno set to ECOMM. */
int epd_initialise(void);

/* Resets the device screen to a white background. */
void epd_reset(void);

/* Put device to sleep (some hardware can be damaged if powered for
   extended periods.

   Returns: 0  Success.
            1  Failed to enter deep sleep mode, errno set to EBUSY. */
int epd_sleep(void);

/* Displays provided bitmap on device display. Bitmap length must
   equal that of the display.

   Returns 0 Success.
           1 Fail, invalid bitmap length, errno set to EINVAL.
           2 Fail, SPI comms, errno set to ECOMM. */
int epd_bitmap_display(uint8_t *bitmap, size_t len);

/* Returns width of device screen in pixels. */
uint16_t epd_get_width(void);

/* Returns height of device screen in pixels. */
uint16_t epd_get_height(void);

#endif /* EPD_H */
