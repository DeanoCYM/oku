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

#include "spi.h"		/* GPIO and SPI communication */
#include "epd.h"		/* Device specific commands */
#include "bitmap.h"		/* Bitmap manipulation */

int main(int argc, char *argv[])
{
    log_info("Testing oku with %s.", argv[argc-1]);

    /* Create bitmap buffer */
    if (bitmap_create())
	goto fail1;
	    
    /* Set some pixels */
    for (uint16_t x = 0; x < epd_get_width(); ++x)
	if (bitmap_px_toggle(x, 0))
	    goto fail2;
    
    for (uint16_t y = 0; y < epd_get_height(); ++y)
	if (bitmap_px_toggle(0, y))
	    goto fail2;

    /* Turn on the device and apply bitmap */
    if (epd_on())
	goto fail2;
    if (epd_display(bitmap_get_raster(), bitmap_get_size()))
	goto fail3;

    /* Clean up */
    if (epd_off())
	goto fail2;

    if (bitmap_destroy())
	goto fail3;

    log_info("Testing complete.");
    return 0;

 fail3:
    epd_off();
 fail2:
    bitmap_destroy();
 fail1:
    log_err("Testing failed");
    return 1;
}
