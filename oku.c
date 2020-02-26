/* oku.c
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

   E-book reader.

*/

#include <ert_log.h>
#include <stdint.h>
#include <stdlib.h>

#include "spi.h"		/* GPIO and SPI communication */
#include "epd.h"		/* Device specific commands */
#include "bitmap.h"		/* Bitmap manipulation */
#include "cpqueue.h"		/* Unicode codepoint buffer queue */
#include "utf8.h"		/* Decode UTF-8 into unicode codepoints */
//#include "text.h"		/* Font rendering */
#include "oku_types.h"		/* Type definitions */

uint8_t binary_pattern[] = 
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
    rectangle->buffer = binary_pattern;

    int err = bitmap_copy(bmp, rectangle, 2, 250);
    if (err > 0)
	return err;
    
    rectangle->buffer = old;
    err = bitmap_destroy(rectangle);

    return err;
}

int
cleanup(EPD *epd, BITMAP *bmp, CP_QUEUE *cpbuf)
{
    return epd_off(epd) || bitmap_destroy(bmp) || cpq_destroy(cpbuf)
	|| epd_destroy(epd);
}

void
die(int err, char *errstr)
{
    log_err("%s", errstr);
    exit(err);
    return;
}

int main(int argc, char *argv[])
{
    enum OKU_ERRNO err = OK;

    /**** PROCESS ARGUEMENTS ****/

    if ( argc < 4 ) {
	printf("%s <textfile> <fontsize> <fontpath>\n", argv[0]);
	return ERR_INPUT;
    }

    char *textpath    = argv[1];
    unsigned fontsize = atoi(argv[2]);
    char *fontpath    = argv[3];

    /**** DEVICE INITIALISATION ****/

    EPD *epd = epd_create();
    err = epd_on(epd);
    if (err > 0)
	die(err, "Failed to start device.");

    BITMAP *bmp = bitmap_create(epd->width, epd->height);
    
    /**** TEXT PROCESSING ****/

    CP_QUEUE *cpbuf = cpq_create(); /* buffer for holding unicode */
    codepoint unicode;
    FILE *utf8 = fopen(textpath, "r");

    while ( utf8_ftocp(utf8, &unicode) != ERR_IO ) {
    	err = cpq_enqueue(cpbuf, unicode);
    	if (err > 0)
    	    die(err, "Enqueuing problem.");
    }

    /**** DISPLAY AND SHUTDOWN ****/

    /* Display bitmap on device */
    err = epd_display(epd, bmp->buffer, bmp->length);
    if (err > 0)
	die(err, "Failed to display bitmap");

    /* Clean up */
    err = cleanup(epd, bmp, cpbuf);
    if (err != 0)
	log_warn("Clean up failed.");

    return err;
}



    /* Draw some features to the device bitmap */
//    err = draw_lines(bmp) || draw_binary_pattern(bmp);
//    if (err > 0)
//	die(err, "Failed to write to bitmap");
    
