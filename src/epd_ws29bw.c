/* epd_ws29bw.c
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
 
   Implementation of epd.h for waveshare 9.2" black and white
   electronic paper display module (EPDM).

   EPDM displays binary bitmap images. Bitmaps are provided as
   pointers to the start of an array of bytes array using bitmap.h.

   One bit defines each pixel across the width of the EPD packed to 8
   a byte, don't care bits are at the end of the row if the device
   width in pixels is not a factor of 8.

   The order of the pixels is left to right. The order of their
   storage within each file byte is most significant bit to least
   significant bit.

   Each bit represents a pixel: 1 is white, 0 is black. This is the
   inverse of the portable bitmap format (PMB) provided by
   bitmap.h. Consequently, before writing bitmap data to ram, bytes
   must be inverted.
 */

#include "epd.h"
#include "spi.h"
#include "oku_types.h"
#include "oku_mem.h"
#include "oku_err.h"

/**********************/
/* Device Information */
/**********************/
#define DEVICE "Waveshare 2.9 B&W"
#define WIDTH  128 		/* Display width (px) */
#define HEIGHT 296		/* Display height (px) */
#define SPI_CHANNEL 0		/* SPI Channel (0 or 1) */
#define SPI_CLK_HZ 32000000	/* SPI clock speed */
#define RESET_DELAY 200		/* GPIO reset pin hold time (ms) */
#define BUSY_DELAY 300		/* GPIO reset pin hold time (ms) */

/****************************/
/* CPP Function like macros */
/****************************/

/* Function like macro: ARRSIZE

   Determines array length (stack only!).

   X - Array declared with memory on stack.

   Returns: size_t size of array in bytes.
*/
#define ARRSIZE(X) (sizeof(X)/sizeof(X[0]))

/*************/
/* Constants */
/*************/

/* BCM GPIO Pin numbers */
enum PIN { PIN_RST  = 17, PIN_DC   = 25,
	   PIN_CS   =  8, PIN_BUSY = 24 };

/* EPD command codes  */
enum COMMAND
    { DRIVER_OUTPUT_CONTROL                  = 0x01,
      BOOSTER_SOFT_START_CONTROL             = 0x0C,
      GATE_SCAN_START_POSITION               = 0x0F,
      DEEP_SLEEP_MODE                        = 0x10,
      DATA_ENTRY_MODE_SETTING                = 0x11,
      SW_RESET                               = 0x12,
      TEMPERATURE_SENSOR_CONTROL             = 0x1A,
      MASTER_ACTIVATION                      = 0x20,
      DISPLAY_UPDATE_CONTROL_1               = 0x21,
      DISPLAY_UPDATE_CONTROL_2               = 0x22,
      WRITE_RAM                              = 0x24,
      WRITE_VCOM_REGISTER                    = 0x2C,
      WRITE_LUT_REGISTER                     = 0x32,
      SET_DUMMY_LINE_PERIOD                  = 0x3A,
      SET_GATE_TIME                          = 0x3B,
      BORDER_WAVEFORM_CONTROL                = 0x3C,
      SET_RAM_X_ADDRESS_START_END_POSITION   = 0x44,
      SET_RAM_Y_ADDRESS_START_END_POSITION   = 0x45,
      SET_RAM_X_ADDRESS_COUNTER              = 0x4E,
      SET_RAM_Y_ADDRESS_COUNTER              = 0x4F,
      TERMINATE_FRAME_READ_WRITE             = 0xFF };

/* 30B Look up table (LUT) for full screen update */
byte lut_full_update[] =
    { 0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
      0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
      0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
      0x35, 0x51, 0x51, 0x19, 0x01, 0x00 };

/* Partial updates not currently implemented */
// const byte lut_partial_update[] =
//	{ 0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
//	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	  0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
//	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/************************/
/* Forward Declarations */
/************************/

static members calculate_pitch(resolution width);

/* Communication with device */
static int init_gpio(void);
static int init_spi(int spi_channel, int spi_clk_hz);
static int wait_while_busy(unsigned int busy_delay);

/* Write to device  */
static int write_command(enum COMMAND command);
static int write_data(byte *data, members len);
static int push_shift_register(void) ;
static int push_lut(byte *lut);

/* Device RAM operations */
static int ram_set_window(coordinate xmin, coordinate xmax,
			  coordinate ymin, coordinate ymax);
static int ram_set_cursor(coordinate x, coordinate y);
static int ram_write(byte *bitmap, coordinate width, coordinate height);
static int ram_load(unsigned int busy_delay);

/***********************/
/* Interface Functions */
/***********************/

/* Function: epd_create()

   Allocate memory for and initialise an EPD structure. Returns a
   handle to the structure. Exits on memory error. */
EPD *
epd_create(void)
{
    EPD *epd = oku_alloc(sizeof *epd); /* exits on failure */

    /* Record device dimensions */
    epd->width  = WIDTH;
    epd->height = HEIGHT;

    /* SPI Communication parameters */
    epd->spi_channel = SPI_CHANNEL;
    epd->spi_clk_hz  = SPI_CLK_HZ;
    epd->reset_delay = RESET_DELAY;
    epd->busy_delay  = BUSY_DELAY;

    /* Stream handle unused in this implementation of epd.h */
    epd->stream = NULL;

    return epd;
}

/* Function: epd_on()

   Fully initialises device, starting SPI communications and setting
   the appropriate pins to the correct modes.

   epd - Electronic paper display device handle. */
int
epd_on(EPD *epd)
{
    int err = OK;

    /* Device Startup Sequence */
    err = init_gpio();		/* sets GPIO pins  */
    if (err > 0) goto out;
    err = init_spi(epd->spi_channel, epd->spi_clk_hz); /* SPI start*/
    if (err > 0) goto out; 
    err = epd_reset(epd);	/* Wipes device display screen */
    if (err > 0) goto out;
    err = push_shift_register(); /* Startup commands */
    if (err > 0) goto out;
    err = push_lut(lut_full_update); /* Sends device lut */
    if (err > 0) goto out;

    /* RAM to hold full bitmap, representing pixels from origin to
       maximum dimensions */
    ram_set_window(0, epd->width, 0, epd->height);

 out:
    return err;
}

/* Function: epd_display()

   Displays provided bitmap on epaper device display. Bitmap length
   must equal that of the display. */
int
epd_display(EPD *epd, byte *bitmap, members len)
{
    members pitch = calculate_pitch(epd->width);

    if (len != pitch * epd->height)
	goto fail1;

    if (ram_write(bitmap, epd->width, epd->height))
	goto fail2;
    if (ram_load(epd->busy_delay))
	goto fail2;

    return OK;
 fail1:
    return ERR_INPUT;
 fail2:
    return ERR_COMMS;
}

/* Function: epd_reset()

   Resets the epaper display screen using the GPIO reset pin, holding
   them at the required level for the device delay time.
*/
int
epd_reset(EPD *epd)
{
    int err = OK;

    err = spi_gpio_write(PIN_RST, GPIO_LEVEL_HIGH);
    if (err > 0) goto out;
    spi_delay(epd->reset_delay);

    err = spi_gpio_write(PIN_RST, GPIO_LEVEL_LOW);
    if (err > 0) goto out;
    spi_delay(epd->reset_delay);

    err = spi_gpio_write(PIN_RST, GPIO_LEVEL_HIGH);
    if (err > 0) goto out;
    spi_delay(epd->reset_delay);

 out:
    return err;
}

/* Function: epd_off()

   Put device into deep sleep. */
int
epd_off(EPD *epd)
{
    int err = OK;
    byte dsm[] = { 0x01 };

    err = wait_while_busy(epd->busy_delay);
    if (err > 0) goto out;

    err = write_command(DEEP_SLEEP_MODE);
    if (err > 0) goto out;
    err = write_data(dsm, ARRSIZE(dsm));

 out:
    return err;
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

/* Static Function: init_gpio()

   Initialises GPIO pins and sets the correct GPIO operating
   mode. Requirements documented in Waveshare manual page 9/26. */
static int
init_gpio()
{
    int err = OK;
    err = spi_init_gpio();	/* often warns about no root */
    if (err < 0)
	return err;
    
    spi_gpio_pinmode(PIN_RST,  SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_DC,   SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_CS,   SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_BUSY, SPI_PINMODE_INPUT);
    
    return err;
}

/* Static function: init_spi()

   spi_channel - SPI communication channel.
   spi_clock_hz - SPI clock speed in Hz.

   Initiates SPI communication with epaper display unit.

*/
static int
init_spi(int spi_channel, int spi_clk_hz)
{
    return spi_open(spi_channel, spi_clk_hz);
}

/* Static function: write_command()

   Sets the GPIO pins for command transfer and transfers given command
   to epaper device. */
static int
write_command(enum COMMAND command)
{
    int err = OK;

    /* DC pin low for command  */
    err = spi_gpio_write(PIN_DC, GPIO_LEVEL_LOW);
    if (err > 0) goto out;
    spi_gpio_write(PIN_CS, GPIO_LEVEL_LOW);
    if (err > 0) goto out;

    /* Ensure that command is on 8-bit byte with bitmask */
    byte byte = command & 0xFF;
    err = spi_write(&byte, 1);
    if (err > 0) goto out;

    err = spi_gpio_write(PIN_CS, GPIO_LEVEL_HIGH);
    
 out:
    return err;
}

/* Function: write_data()

   Sets the GPIO pins for data transfer and transfers given
   data to epaper device. */
static int
write_data(byte *data, members len)
{
    int err = OK;

    /* DC pin high for data transfer, CS low. */
    err = spi_gpio_write(PIN_DC, GPIO_LEVEL_HIGH);
    if (err > 0) goto out;
    err = spi_gpio_write(PIN_CS, GPIO_LEVEL_LOW);
    if (err > 0) goto out;

    /* Write data */
    err = spi_write(data, len);
    if (err > 0) goto out;

    /* Reset chip select when write complete */
    err = spi_gpio_write(PIN_CS, GPIO_LEVEL_HIGH);
    
 out:
    return err;
}

/* Static function: push_shift_register.

   Write the required data to the device shift register. */
static int
push_shift_register(void) 
{
    int err = OK;

    /* Device specific data for commands in COMMAND enum, data
       variable name abbreviations correspond to COMMAND enum */
    byte doc[]  = { (HEIGHT-1) & 0xFF, ((HEIGHT-1) >> 8) & 0xFF, 0x00 };
    byte bssc[] = { 0xD7, 0xD6, 0x9D };
    byte wvr[]  = { 0xA8 };
    byte sdlp[] = { 0x1A };
    byte sgt[]  = { 0x08 };
    byte bwc[]  = { 0x03 };
    byte dems[] = { 0x03 };

    err = write_command(DRIVER_OUTPUT_CONTROL);
    if (err > 0) goto out;
    err = write_data(doc, ARRSIZE(doc));
    if (err > 0) goto out;
    err = write_command(BOOSTER_SOFT_START_CONTROL);
    if (err > 0) goto out;
    err = write_data(bssc, ARRSIZE(bssc));
    if (err > 0) goto out;
    err = write_command(WRITE_VCOM_REGISTER);
    if (err > 0) goto out;
    err = write_data(wvr, ARRSIZE(wvr));
    if (err > 0) goto out;
    err = write_command(SET_DUMMY_LINE_PERIOD);
    if (err > 0) goto out;
    err = write_data(sdlp, ARRSIZE(sdlp));
    if (err > 0) goto out;
    err = write_command(SET_GATE_TIME);
    if (err > 0) goto out;
    err = write_data(sgt, ARRSIZE(sgt));
    if (err > 0) goto out;
    err = write_command(BORDER_WAVEFORM_CONTROL);
    if (err > 0) goto out;
    err = write_data(bwc, ARRSIZE(bwc));
    if (err > 0) goto out;
    err = write_command(DATA_ENTRY_MODE_SETTING);
    if (err > 0) goto out;
    err = write_data(dems, ARRSIZE(dems));
 out:
    return err;
}

/* Static Function: push_lut

   Send 30B look up table to device. */
static int
push_lut(byte *lut)
{
    int err = OK;

    err = write_command(WRITE_LUT_REGISTER);
    if (err > 0) goto out;
    err = write_data(lut, 30);
 out:
    return err;
}
    
/* Static Function: wait_while_busy()

   Pauses process until busy pin reads low. If the waiting time is
   greater than 100 x busy_delay seconds, returns appropriate error
   code.

   busy_delay - delay time (s) */

static int
wait_while_busy(unsigned int busy_delay)
{
    int t = 0;
    while (spi_gpio_read(PIN_BUSY) == GPIO_LEVEL_HIGH) {
	spi_delay(busy_delay);

	if (++t > 100)
	    return ERR_BUSY;
    }
	
    return OK;
}

/* Static function: ram_set_window.

   Sets the RAM address start and end positions for the device. The range,
   and its corresponding area, determines the e-paper display
   window. Parameters should provided as pixel coordinates from origin of
   display.

   xmin,xmax,ymin,ymax - Cartesian coordinates from origin to extrema. */
static int
ram_set_window(coordinate xmin, coordinate xmax,
	       coordinate ymin, coordinate ymax)
{
    int err = OK;

    /* One byte of data holds information for 8 pixels across the
       width. So 8px of data requires 1B of RAM, hence division by 8
       required (x >> 3) */

    // Does not account for sizes that are not factor of eight, but
    // this is not an issue for this device
    byte x_start_end[] = { (xmin >> 3) & 0xFF, (xmax >> 3) & 0xFF };

    /* Pixel height can be greater than 255 (more than 1B) on this
       device. Masking required to send as two bytes. */
    byte y_start_end[] = { ymin & 0xFF, (ymin >> 8) & 0xFF,
			   ymax & 0xFF, (ymax >> 8) & 0xFF };

    err = write_command(SET_RAM_X_ADDRESS_START_END_POSITION);
    if (err > 0) goto out;
    err = write_data(x_start_end, ARRSIZE(x_start_end));
    if (err > 0) goto out;
    err = write_command(SET_RAM_Y_ADDRESS_START_END_POSITION);
    if (err > 0) goto out;
    err = write_data(y_start_end, ARRSIZE(y_start_end));

 out:
    return err;
}
/* Static function: ram_set_cursor.

   Sets the RAM cursor for next bitmap write. Takes coordinates that
   represent number of pixels from display origin. */
static int 
ram_set_cursor(coordinate x, coordinate y)
{
    int err = OK;

    /* One byte of data holds information for 8 pixels across the
       width. So 8px of data requires 1B of RAM, hence division by 8
       required (x >> 3) */
    byte x_ram_start[] = { (x >> 3) & 0xFF };

    /* Pixel height can be greater than 255 (more than 1B) on this
       device. Masking required to send as two bytes. */
    byte y_ram_start[] = { y & 0xFF, (y >> 8) & 0xFF };

    err = write_command(SET_RAM_X_ADDRESS_COUNTER);
    if (err > 0) goto out;
    err = write_data(x_ram_start, ARRSIZE(x_ram_start));
    if (err > 0) goto out; 
    err = write_command(SET_RAM_Y_ADDRESS_COUNTER);
    if (err > 0) goto out;
    err = write_data(y_ram_start, ARRSIZE(y_ram_start));

 out:
    return err;
}

/* Static function ram_write()

   Write the provided bitmap to the device RAM row by row, as is
   required by this device. Device representation of black is opposite
   to that in bitmap.h so the byte needs to be inverted bitwise. */
static int
ram_write(byte *bitmap, coordinate width, coordinate height)
{
    int err = OK;

    for (coordinate y = 0; y < height; ++y) {
	/* Set the cursor at the start of each row. */
	err = ram_set_cursor(0, y);
	if (err > 0) goto out;
	err = write_command(WRITE_RAM);
	if (err > 0) goto out;

	/* Each byte holds data for 8 pixels in a row. */
	for (members x = 0; x < calculate_pitch(width); ++x, bitmap++) {
	    byte inverted = ~(*bitmap);
	    err = write_data(&inverted, 1);
	    if (err > 0) goto out;
	}
    }
 out:
    return err;
}

/* Static function: ram_load.

   Load the bitmap stored in the RAM window, defined by
   ram_set_window() to the epaper device display. */
static int
ram_load(unsigned int busy_delay)
{
    int err = OK;

    byte duc2[] = { 0xC4 };

    err = write_command(DISPLAY_UPDATE_CONTROL_2);
    if (err > 0) goto out;
    err = write_data(duc2, ARRSIZE(duc2));
    if (err > 0) goto out;
    err = write_command(MASTER_ACTIVATION);
    if (err > 0) goto out;
    err = write_command(TERMINATE_FRAME_READ_WRITE);
    if (err > 0) goto out;
    err = wait_while_busy(busy_delay);

 out:
    return err;
}

/* Static Function: calculate_pitch()

   Returns the number of bytes required to define each pixels across
   the width.

   1 byte can represent 8px across the width. However, an extra byte
   is required on each row if the number of px across the width is not
   a factor of 8.  */
static members
calculate_pitch(resolution width)
{
    return (width % 8 == 0) ? width / 8: width / 8 + 1;
}
