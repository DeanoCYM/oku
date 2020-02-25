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

#include <stdio.h>		/* FILE * */

#include "oku_types.h"

/***********/
/* Objects */
/***********/

/* Object: EPD

   Electronic paper device (EPD) structure. */
typedef struct EPD {
    /* Device dimensions */
    resolution width;		/* Device pixel count in width */
    resolution height;		/* Device pixel count  */
    /* Communication parameters */
    int spi_channel;		/* SPI Channel (0 or 1) */
    int spi_clk_hz;		/* SPI clock speed */
    unsigned int reset_delay;	/* GPIO reset pin hold time (ms) */
    int busy_delay;		/* GPIO reset pin hold time (ms) */
    FILE *stream;		/* Implementation specific handle */
} EPD;

/*************/
/* Interface */
/*************/

/* All functions, unless otherwise documented, return 0 on success or
   the error codes defined in oku_err.h on failure. Negative values
   indicate warnings. */

/* Function: epd_create()

   Allocates memory for electronic paper display object, initialises
   values within that object and returns a handle to it. */
EPD *epd_create(void);

/* Function: epd_on()

   Sends commands and data to start the device. Must be performed
   prior to any other routines on the device handle, as well as to
   wake from sleep. Device remains powered and initialised!  Use
   epd_off() to ensure device is not powered for long period of time
   as this can damage some devices. */
int epd_on(EPD *epd);

/* Function: epd_display()

   Displays provided bitmap on device display. Bitmap length must
   equal that of the display. Device remains powered and initialised!
   Use epd_off() to ensure device is not powered for long period of
   time as this can damage some devices.

   bitmap - A pointer to buffer containing image data (see bitmap.h,
   particularly struct BITMAP). The first bit is the origin at the top
   left corner of the device. The order of their storage within each
   byte is most significant bit to least significant bit. The bitmap
   proceeeds from left to right, packed 8 to a byte, don't care bits
   to fill out the last byte in the row if the width is not a factor
   of 8.  Each bit represents a pixel: 1 is black, 0 is white.

   len - 1 dimensional length of bitmap in bytes. */
int epd_display(EPD *epd, byte *bitmap, size_t len);

/* Function: epd_reset()

   Resets the device screen to a white background. The device remains
   active as if epd_on() has been run. */
int epd_reset(EPD *epd);

/* Function: epd_off()

   Put device into a safe sleep state, note some hardware can be
   damaged if powered on for extended periods. */
int epd_off(EPD *epd);

/* Function epd_destroy()

   Free all memory associated with object.*/
int epd_destroy(EPD *epd);

#endif /* EPD_H */
