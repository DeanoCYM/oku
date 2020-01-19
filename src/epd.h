/* epd.h
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

/* Communication with display device. Requires hardware specific
   implementation.
*/

#ifndef EPD_H
#define EPD_H

#include <sys/types.h>
#include <stdint.h>

/***********/
/* Objects */
/***********/

/* Object: EPD

   Electronic paper device (EPD) structure.
*/
typedef struct EPD {
    int black_colour;		/* logical representation of black pixel */
    uint16_t width;		/* Device pixel count in width */
    uint16_t height;		/* Device pixel count  */
    int spi_channel;		/* SPI Channel (0 or 1) */
    int spi_clk_hz;		/* SPI clock speed */
    unsigned int reset_delay;	/* GPIO reset pin hold time (ms) */
    int busy_delay;		/* GPIO reset pin hold time (ms) */
} EPD;

/*************/
/* Interface */
/*************/

/* Function: epd_on

   Fully initialises the device. Must be performed prior to any other
   epd routines as well as to wake from sleep. Device remains powered
   and initialised!  Use epd_off() to ensure device is not powered for
   long period of time as this can damage some devices.

   epd - Electronic paper display handle.

   Returns:
   0  Success, device initialised.
   1  Fail, errno set to ECOMM.
*/
int epd_on(EPD *epd);

/* Function: epd_display

   Displays provided bitmap on device display. Bitmap length must
   equal that of the display. Device remains powered and initialised!
   Use epd_off() to ensure device is not powered for long period of
   time as this can damage some devices.

   epd - Electronic paper display object.

   bitmap - A pointer to buffer containing image data (see bitmap.h,
   particularly struct BITMAP). The first bit is the origin at the top
   left corner of the device. The order of their storage within each
   byte is most significant bit to least significant bit. The bitmap
   proceeeds from left to right, packed 8 to a byte, don't care bits
   to fill out the last byte in the row if the width is not a factor
   of 8.  Each bit represents a pixel: 1 is black, 0 is white.

   len - 1 dimensional length of bitmap in bytes.

   Returns:
   0 Success.
   1 Fail, invalid bitmap length, errno set to EINVAL.
   2 Fail, communication error, errno set to ECOMM.
*/
int epd_display(EPD *epd, uint8_t *bitmap, size_t len);

/* Function: epd_reset

   Resets the device screen to a white background. The device remains
   active as if epd_on() has been run.

   epd - Electronic paper display object.
*/
int epd_reset(EPD *epd);

/* Function: epd_off

   Put device into a safe sleep state, note some hardware can be
   damaged if powered on for extended periods.

   epd - Electronic paper display object.

   Returns:
   0 Success.
   1 Failed to reset, errno set to EBUSY or ECOMM.
*/
int epd_off(EPD *epd);

#endif /* EPD_H */
