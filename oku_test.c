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

/* Function: intitialise_display_bitmap()

   Initialises a bitmap buffer to the correct dimensions for
   electronic paper display device. The bitmap is cleared.

*/
int
initialise_display_bitmap(struct EPD *epd, struct BITMAP *bmp)
{
    if ( bitmap_create(bmp, epd->width, epd->height) )
	return 1;

    bmp->buffer = malloc(bmp->length);
    if (!bmp->buffer)
	return 1;

    if ( bitmap_clear(bmp, epd->black_colour) )
	return 1;

    return 0;
}

/* Function: draw_lines()

   Draws dotted lines to the device bitmap. */
int
draw_lines(struct BITMAP *bmp, struct EPD *epd)
{
    for (coordinate x = 0; x < bmp->width; x += 5)
    	for (coordinate y = 0; y < bmp->length / bmp->pitch ; y += 5)
    	    if ( bitmap_modify_px(bmp, x, y,
    				  SET_PIXEL_BLACK, epd->black_colour) )
		return 1;

    return 0;
}

/* Function: draw_binary_pattern()

   Draw the binary pattern to the device */
int
draw_binary_pattern(struct BITMAP *bmp)
{
    BITMAP rectangle;
    if ( !bitmap_create(&rectangle, 2 * 8, 16) )
	return 1;
	
    uint8_t rect[] = { 0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
		       0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
		       0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
		       0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F };
    rectangle.buffer = rect;

    if ( bitmap_copy(bmp, &rectangle, 2, 250) )
	return 1;

    return 0;
}

int main(int argc, char *argv[])
{
    log_info("Testing oku with %s.", argv[argc-1]);

    /* Communication with display hardware is performed using the EPD
       object and epd.h interface. */
    EPD epd;
    if ( epd_on(&epd) )		/* hardware initialisation */
	exit(1);

    /* Create a bitmap for the display device */
    BITMAP bmp;
    if ( initialise_display_bitmap(&epd, &bmp) ) {
	log_err("Failed to initialise bitmap");
	exit(1);
    }
    
    /* Draw some features to the device bitmap */
    if ( draw_lines(&bmp, &epd) || draw_binary_pattern(&bmp) ) {
	log_err("Failed to write to bitmap");
	exit (1);
    }

    
    /* Write some text. */
    /* char font[] = "/usr/share/fonts/truetype/dejavu/DejaVuSansCondensed.ttf"; */
    /* char textfile[] = "./utf8_string"; */

    /* struct TEXTBOX txt; */

    /* txt.font_file = font; */
    /* txt.font_size = 30; */
    /* codepoint string[] = { 'Z', 'e', 'y', 'n', 'e', 'p', ' ', */
    /* 			   'a', 'n', 'd', ' ', 'G', 0xFC, 'n', 'e', 0x015F, ' ', */
    /* 			   'H', 'e', 'l', 'l', 'o', ' ', 't', */
    /* 			   'h', 'i', 's', ' ', 'i', 's', ' ', */
    /* 			   'm', 'y', ' ', '2', 'n', 'd', ' ', */
    /* 			   't', 'e', 's', 't', '!', 0x263A, 0x263A }; */

    /* txt.length = sizeof string / sizeof *string; */
    /* txt.utf_str = string; */
    /* txt.bmp = &bmp; */

    /* if ( textbox_init(&txt) ) */
    /* 	exit(1); */

    /* textbox_write(&txt);\ */


    /* Display bitmap on device */
    if ( epd_display(&epd, bmp.buffer, bmp.length) )
	exit(1);

    /* Clean up */
    //    textbox_close(&txt);
    free(bmp.buffer);
    bmp.buffer = NULL;

    if ( epd_off(&epd) )
	exit(1);

    log_debug("Testing complete.");

    return 0;
}

