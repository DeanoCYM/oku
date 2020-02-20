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
#include "oku_types.h"

/**********************/
/* Device Information */
/**********************/
#define DEVICE "PBM Emulator"	 /* Device name */
#define FILENAME "./display.pbm" /* PBM file path */
#define WIDTH  128		 /* Display width (px) */
#define HEIGHT 296		 /* Display height (px) */
#define STR_LEN 6		 /* Number of characters in width+height */

/********************/
/* Global Variables */
/********************/
FILE *pbm = NULL;		/* PMB file handle */

/************************/
/* Forward Declarations */
/************************/
static int file_open(const char *filename);
static int file_close(void);
static int check_pbm(void);
static int write_headers(void);
static int write_bitmap(byte *bitmap, size_t len);

/*************/
/* Interface */
/*************/

/* Function: epd_on

   Opens new PBM file and writes headers in anticipation of later
   binary buffer write.

   epd - Electronic paper display object.

   Returns:
   0  Success, device initialised.
   1  Fail, file I/O error (errno ECOMM).
*/
int
epd_on(EPD *epd)
{
    log_info("Starting %s.", DEVICE);
    
    /* In a pbm file, a black pixel is represented by logical 1. */
    epd->black_colour = 1;
    epd->width = WIDTH;
    epd->height = HEIGHT;
    epd->spi_channel = 0;
    epd->spi_clk_hz = 0;
    epd->reset_delay = 0;
    epd->busy_delay = 0;

    if (pbm) {			/* file already open */
	log_err("File %s already open", FILENAME);
	goto fail1;
    }
    
    if ( file_open(FILENAME) ) {
	log_err("Cannot open file %s.", FILENAME);
	goto fail1;
    }

    if ( write_headers() ) {
	log_err("Cannot write headers to file %s.", FILENAME);
	goto fail2;
    }

    return 0;
 fail2:
    file_close();	    /* File closed and handle reset to NULL */
 fail1:
    errno = ECOMM;
    return 1;
}

/* Function: epd_display

   Writes binary image data to file.

   epd - Electronic paper display object.

   bitmap - Pointer to bitmap buffer.

   len - Length of bitmap in buffer in bytes.

   Returns:
   0 Success.
   1 Fail, invalid bitmap length, (errno EINVAL).
   2 Fail, communication error, (errno ECOMM).
*/
int
epd_display(EPD *epd __attribute__((unused)),
	    byte *bitmap, size_t len)
{
    log_info("Displaying %luB bitmap.", len);

    if (len == 0 || bitmap == NULL ) {
	log_err("Invalid bitmap.");
	errno = EINVAL;
	return 1;
    }

    if ( write_bitmap(bitmap, len) ) {
	log_err("Failed to write to %s.", FILENAME);
	errno = ECOMM;
	return 2;
    }
       
    return 0;
}


/* Function epd_reset.

   Opens and closes file (overwriting contents).

   Returns:
   0 Success.
   1 Failed to reset (errno set to EBUSY or ECOMM).

*/
int
epd_reset(EPD *epd)
{
    log_info("Resetting %s.", DEVICE);

    return epd_on(epd) || epd_off(epd);
}

/* Closes PBM file */
int
epd_off(EPD *epd __attribute__((unused)))
{
    log_info("Device %s entering sleep mode.", DEVICE);

    if (file_close()) {
	log_err("Failed to close file %s", FILENAME);
	errno = EBUSY;
	return 1;
    }

    return 0;
}

/********************/
/* Static Functions */
/********************/

/* Static function: file_open

   Opens file for writing PBM image.

   filname - path to pbm file.

   Returns: 0 Success.
            1 Failed to open file. */
static int
file_open(const char *filename)
{
    pbm = fopen(filename, "w");
    if (check_pbm())
	return 1;

    return 0;
}

/* Static function: file_close

   Closes PBM image file.

   Returns: 0 Success.
            1 Invalid file pointer.
            2 Failed to close file. */
static int
file_close(void)
{
    log_info("Closing file %s for device %s.", FILENAME, DEVICE);

    if (check_pbm())
	return 1;

    if (fclose(pbm)) {
	log_warn("Failed to close file %s.", FILENAME);
	return 2;
    }

    pbm = NULL;

    return 0;
}

/* Static function: write_headers

   Writes PBM headers to file.

   PBM starts with the two characters "P4", followed by whitespace
   (blanks, TABs, CRs, LFs).

   The width in pixels of the image, formatted as ASCII characters
   in decimal, followed by whitespace.
      
   The height in pixels of the image, again in ASCII decimal, followed
   by whitespace character (usually a newline).

   Returns:
   0 Success, (at least some characters written).
   1 Fail, no characters written.
*/
static int
write_headers(void)
{
    /* Length of the string written is checked against return value of
       fprintf(). 5 characters, plus the device dimensions are used
       including the newline. */
    int len = 5 + STR_LEN;
    return fprintf(pbm, "P4 %d %d\n", WIDTH, HEIGHT) != len ? 1 : 0;
}

/* Static function: write bitmap

   Writes binary bitmap in format required by PBM files:

   A raster of Height rows, in order from top to bottom.

   Each row is Width bits, packed 8 to a byte, with don't care bits to
   fill out the last byte in the row.

   Each bit represents a pixel: 1 is black, 0 is white.

   The order of the pixels is left to right. The order of their
   storage within each file byte is most significant bit to least
   significant bit. The order of the file bytes is from the beginning
   of the file toward the end of the file.

   Returns 0 Success.
           1 Fail, SPI comms.
*/
static int
write_bitmap(byte *bitmap, size_t len)
{
    size_t res = 0;		/* bytes written */
    
    res = fwrite(bitmap, sizeof bitmap[0], len, pbm);
    if (res < len) {
	log_err("Incomplete write to %s", FILENAME);
	return 1;
    }

    return 0;
}

/* Static function: check_pbm

   Check file pointer is not NULL.

   Returns:
   0 not NULL.
   1 NULL.
   */
static int
check_pbm(void)
{
    if (!pbm) {
	log_warn("Invalid file pointer.");
	return 1;
    }

    return 0;
}
