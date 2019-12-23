/* epd_emulated.c
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
 * Portable bit map (PBM) implementation of epd.h. Emulated electronic
 * paper device screen for testing/debugging.
 * 
 */

#include <ert_log.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>

#include "epd.h"

#define DEVICE "PBM Emulator"
#define FILENAME "./display.pbm"
#define WIDTH  128 		/* Display width (px) */
#define HEIGHT 296		/* Display height (px) */

FILE *pbm = NULL;

/* Forward Declarations */
static int file_open(const char *filename);
static int file_close(void);
static int check_pbm(void);
static int write_headers(void);
static int write_bitmap(uint8_t *bitmap, size_t len);

/* Interface */

/* Opens new PBM file and writes headers. */
int
epd_on(void)
{
    log_info("Starting %s.", DEVICE);
    
    if (pbm) {			/* file already open */
	errno = EALREADY;
	log_warn("File %s already open", FILENAME);
	return 1;
    }
    
    if (file_open(FILENAME)) {
	log_err("Cannot open file %s.", FILENAME);
	errno = ECANCELED;
	goto fail1;
    }

    if (write_headers()) {
	log_err("Cannot write headers to file %s.", FILENAME);
	errno = ECANCELED;
	goto fail2;
    }

    return 0;
 fail2:
    file_close();
 fail1:
    log_err("Failed to initialise device %s.", DEVICE);
    errno = ECANCELED;
    return 1;

}

/* Displays provided bitmap on device display. Bitmap length must
   equal that of the display.

   Returns 0 Success.
           1 Fail, invalid bitmap length, errno set to EINVAL.
           2 Fail, SPI comms, errno set to ECOMM. */
int
epd_display(uint8_t *bitmap, size_t len)
{
    log_info("Displaying %zuB bitmap.", len);

    int res = write_bitmap(bitmap, len);
    switch (res) {
    case 0:
	break;
    case 1:
	log_err("Invalid bitmap length.");
	errno = EINVAL;
	break;
    case 2:
	log_err("Failed to write to %s", FILENAME);
	errno = ECOMM;
	break;
    default:			/* never reach */
	log_err("Unknown error code.");
	
    }
       
    return res;
}


/* Opens and closes file (overwriting contents). */
void
epd_reset(void)
{
    log_info("Resetting %s.", DEVICE);

    epd_off();
    epd_on();

    return;
}

/* Closes PBM file */
int
epd_off(void)
{
    log_info("Device %s entering sleep mode.", DEVICE);

    if (file_close()) {
	log_err("Failed to close file %s", FILENAME);
	errno = EBUSY;
	return 1;
    }

    return 0;
}

/* Returns width of image in pixels */
uint16_t
epd_get_width(void)
{
    return WIDTH;
}

/* Returns height of image in pixels */
uint16_t
epd_get_height(void)
{
    return HEIGHT;
}

/*** Static Functions ***/

/* Opens file for writing PBM image.

   Returns: 0 Success.
            1 Failed to open file, errno set to ECANCELED */
static int
file_open(const char *filename)
{
    pbm = fopen(filename, "w");
    if (check_pbm())
	return 1;

    return 0;
}

/* Closes PBM image file.

   Returns: 0 Success.
            1 Invalid file pointer, errno set to ECANCELED.
            2 Failed to close file, errno set to ECANCELED. */
static int
file_close(void)
{
    log_info("Closing file %s for device %s.", FILENAME, DEVICE);

    if (check_pbm())
	return 1;

    if (fclose(pbm)) {
	log_warn("Failed to close file %s.", FILENAME);
	errno = ECANCELED;
	return 2;
    }

    pbm = NULL;

    return 0;
}

/* Check file pointer is not NULL.

   Returns: 0 not NULL.
            1 NULL, errno set to ECANCELED.
   */
static int
check_pbm(void)
{
    if (!pbm) {
	log_err("Invalid file pointer.");
	errno = ECANCELED;
	return 1;
    }

    return 0;
}

/* Writes PBM headers to file.

   PBM starts with the two characters "P4", followed by whitespace
   (blanks, TABs, CRs, LFs).

   The width in pixels of the image, formatted as ASCII characters
   in decimal, followed by whitespace.
      
   The height in pixels of the image, again in ASCII decimal, followed
   by whitespace character (usually a newline). */
static int
write_headers(void)
{
    fprintf(pbm, "P4 %d %d\n", WIDTH, HEIGHT);
    return 0;
}

/* Writes binary bitmap in format required by PBM files:

   A raster of Height rows, in order from top to bottom.

   Each row is Width bits, packed 8 to a byte, with don't care bits to
   fill out the last byte in the row.

   Each bit represents a pixel: 1 is black, 0 is white.

   The order of the pixels is left to right. The order of their
   storage within each file byte is most significant bit to least
   significant bit. The order of the file bytes is from the beginning
   of the file toward the end of the file.

   Returns 0 Success.
           1 Fail, invalid bitmap length, errno set to EINVAL.
           2 Fail, SPI comms, errno set to ECOMM. */
static int
write_bitmap(uint8_t *bitmap, size_t len)
{
    size_t res = 0;		/* bytes written */
    
    res = fwrite(bitmap, sizeof bitmap[0], len, pbm);
    if (res < len) {
	log_err("Incomplete write to %s", FILENAME);
	return 1;
    }

    return 0;
}
