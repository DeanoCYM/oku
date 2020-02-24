/* oku_test.c
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

   Functional test of oku.

*/

#include <ert_log.h>
#include <stdint.h>
#include <stdlib.h>

#include "spi.h"		/* GPIO and SPI communication */
#include "epd.h"		/* Device specific commands */
#include "bitmap.h"		/* Bitmap manipulation */
#include "text.h"		/* Font rendering */

uint8_t rect[] = 
    { 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
      0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
      0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
      0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F };


/* Function: draw_lines()

   Draws dotted lines to the device bitmap. */
int
draw_lines(struct BITMAP *bmp)
{
    for (coordinate x = 0; x < bmp->width; x += 5)
    	for (coordinate y = 0; y < bmp->length / bmp->pitch ; y += 5) {

    	    int err = bitmap_modify_px(bmp, x, y, SET_PIXEL_BLACK);
	    if (err > 0)
		return err;
	}

    return OK;
}

/* Function: draw_binary_pattern()

   Draw the binary pattern to the device */
int
draw_binary_pattern(struct BITMAP *bmp)
{
    BITMAP *rectangle = bitmap_create(2 * 8, 16);

    byte *old = rectangle->buffer;
    rectangle->buffer = rect;

    int err = bitmap_copy(bmp, rectangle, 2, 250);
    if (err > 0)
	return err;
    
    rectangle->buffer = old;
    err = bitmap_destroy(rectangle);

    return err;
}

int main(int argc, char *argv[])
{
    log_info("Testing oku with %s.", argv[argc-1]);
    log_debug("Running %s in degbug mode.", argv[argc-1]);

    enum OKU_ERRNO err = OK;

    /* Communication with display hardware is performed using the EPD
       object and epd.h interface. */
    EPD *epd = epd_create();
    err = epd_on(epd);
    if (err > 0) {
	log_err("Failed to start device.");
	exit(err);
    }

    /* Create a bitmap for the display device */
    BITMAP *bmp = bitmap_create(epd->width, epd->height);
    
    /* Draw some features to the device bitmap */
    err = draw_lines(bmp) || draw_binary_pattern(bmp);
    if (err > 0) {
	log_err("Failed to write to bitmap");
	exit(err);
    }
    
    /* Display bitmap on device */
    err = epd_display(epd, bmp->buffer, bmp->length);
    if (err > 0) {
	log_err("Failed to display bitmap");
	exit(err);
    }

    /* Clean up */
    err = epd_off(epd) || bitmap_destroy(bmp) || epd_destroy(epd);
    if (err > 0)
	log_warn("Clean up failed.");

    return err;
}

