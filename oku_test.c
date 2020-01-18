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

int main(int argc, char *argv[])
{
    log_info("Testing oku with %s.", argv[argc-1]);

    /* Communication with display hardware is performed using the EPD
       object and epd.h interface. */
    EPD epd;
    if ( epd_on(&epd) )		/* hardware initialisation */
	exit(1);

    /* Bitmap management is performed using the BITMAP object and
       bitmap.h. The bitmap->buffer field contains the image buffer,
       and so can be large - it must be allocated manually. */
    BITMAP bmp;
    if ( bitmap_create(&bmp, BLACK) ) /* match bitmap fields to device */
	exit(1);

    bmp.buffer = malloc(bmp.length); /* large image buffer */
    if (!bmp.buffer) {
	log_err("Memory error.");
	exit(1);
    }
    
    if ( bitmap_clear(bmp) ) 	/* wipe the buffer */
	exit(1);

    /* Set some pixels */
    for (uint16_t y = 0; y < bitmap.height; y += 2)
	for (uint16_t x = 0; x < bitmap.width; x += 2)
	    if ( bitmap_px_toggle(bitmap, x, y) )
		exit(1);
    
    /* Display bitmap on device*/
    if ( epd_display(epd, bitmap->buffer) )
	exit(1);
	    
    /* Clean up */
    if ( epd_off(epd) )
	exit(1);

    if ( bitmap_destroy(bitmap) )
	exit(1);

    log_info("Testing complete.");

    return 0;
}
