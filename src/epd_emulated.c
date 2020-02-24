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

#include "epd.h"
#include "oku_types.h"
#include "oku_mem.h"

/**********************/
/* Device Information */
/**********************/
#define DEVICE "PBM Emulator"	 /* Device name */
#define FILENAME "./display.pbm" /* PBM file path */
#define WIDTH  128		 /* Display width (px) */
#define HEIGHT 296		 /* Display height (px) */
#define STR_LEN 6		 /* Number of characters in width+height */

/************************/
/* Forward Declarations */
/************************/
static int file_open(const char *filename, EPD *epd);
static int file_close(EPD *epd);
static int file_check(FILE *pbm);
static int file_write_headers(FILE *pbm);
static int file_write_bitmap(byte *bitmap, members len, FILE *pbm);

/*************/
/* Interface */
/*************/

/* Function: epd_create()

   Allocate memory for and initialise an EPD structure. Returns a
   handle to the structure.

   For this software only implementation, SPI communication fields are
   not required. */
EPD *
epd_create(void)
{
    EPD *epd = oku_alloc(sizeof *epd);

    /* Record device dimensions */
    epd->width  = WIDTH;
    epd->height = HEIGHT;

    return epd;
}

/* Function: epd_on()

   Opens new PBM file and writes headers in anticipation of later
   binary buffer write. */
int
epd_on(EPD *epd)
{
    int err = file_open(FILENAME, epd);
    if (err > 0)
	return err;

    err = file_write_headers(epd->stream);
    if (err > 0) {
	file_close(epd);
	return err;
    }

    return OK;
}
 
/* Function: epd_display()

   Writes binary image data to file.

   bitmap - Pointer to bitmap buffer.
   len - Length of bitmap in buffer in bytes. */
int
epd_display(EPD *epd __attribute__((unused)), byte *bitmap, members len)
{
    if (len == 0 || bitmap == NULL)
	return ERR_INPUT;

    return file_write_bitmap(bitmap, len, epd->stream);
}

/* Function: epd_reset()

   Opens and closes file, deletes all contents and rewrites
   headers. */
int
epd_reset(EPD *epd)
{
    return epd_on(epd) || epd_off(epd);
}

/* Closes PBM file */
int
epd_off(EPD *epd)
{
    return file_close(epd);
}

int
epd_destroy(EPD *epd)
{
    if (epd == NULL)
	return ERR_UNINITIALISED;

    oku_free(epd);

    return OK;
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
file_open(const char *filename, struct EPD *epd)
{
    epd->stream = fopen(filename, "w");

    return file_check(epd->stream) ? ERR_IO : OK;
}

/* Static function: file_close

   Closes PBM image file. */
static int
file_close(struct EPD *epd)
{
    int err = OK;
    err = file_check(epd->stream);
    if (err > 0) return err;

    err = fclose(epd->stream);
    epd->stream = NULL;

    return err;
}

/* Static function: write_headers

   Writes PBM headers to file.

   PBM starts with the two characters "P4", followed by whitespace
   (blanks, TABs, CRs, LFs).

   The width in pixels of the image, formatted as ASCII characters
   in decimal, followed by whitespace.
      
   The height in pixels of the image, again in ASCII decimal, followed
   by whitespace character (usually a newline). */
static int
file_write_headers(FILE *pbm)
{
    /* Length of the string written is checked against return value of
       fprintf(). 5 characters, plus the device dimensions are used
       including the newline. */
    int len = 5 + STR_LEN;
    return fprintf(pbm, "P4 %d %d\n", WIDTH, HEIGHT) != len
	? ERR_PARTIAL_WRITE : OK;
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
   of the file toward the end of the file. */
static int
file_write_bitmap(byte *bitmap, members len, FILE *pbm)
{
    return fwrite(bitmap, sizeof *bitmap, len, pbm) < len
	? ERR_PARTIAL_WRITE : OK;
}

/* Static function: file_check()

   Check file pointer is not NULL. */
static int
file_check(FILE *pbm)
{
    return pbm == NULL ? ERR_UNINITIALISED : OK;
}
