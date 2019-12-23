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
   uint8_t* arrays using bitmap.h.

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

#include <ert_log.h>
#include <errno.h>
#include <stdint.h>

/* Device information */
#define DEVICE "Waveshare 2.9 B&W"
#define WIDTH  128 		/* Display width (px) */
#define HEIGHT 296		/* Display height (px) */
#define SPI_CHANNEL 0		/* SPI Channel (0 or 1) */
#define SPI_CLK_HZ 32000000	/* SPI clock speed */
#define RESET_DELAY 200		/* GPIO reset pin hold time (ms) */
#define BUSY_DELAY 300		/* GPIO reset pin hold time (ms) */

/* Determines array length (stack only!) */
#define ARRSIZE(X) (sizeof(X)/sizeof(X[0]))

/* BCM GPIO Pin numbers */
enum PIN { PIN_RST  = 17, PIN_DC   = 25,
	   PIN_CS   =  8, PIN_BUSY = 24 };

/* GPIO output voltage */
enum GPIO_LEVEL {GPIO_LEVEL_LOW, GPIO_LEVEL_HIGH};

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
uint8_t lut_full_update[] =
    { 0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
      0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
      0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
      0x35, 0x51, 0x51, 0x19, 0x01, 0x00 };

/* Partial updates not currently implemented */
// const uint8_t lut_partial_update[] =
//	{ 0x10, 0x18, 0x18, 0x08, 0x18, 0x18, 0x08, 0x00,
//	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//	  0x00, 0x00, 0x00, 0x00, 0x13, 0x14, 0x44, 0x12,
//	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/*** Forward Declarations ***/
static int init_gpio(void);
static int init_spi(void);
static int write_command(enum COMMAND command);
static int write_data(uint8_t *data, size_t len);
static int push_shift_register(void);
static int push_lut(uint8_t *lut);
static int wait_while_busy(void);
static int ram_set_window(uint16_t xmin, uint16_t xmax,
			  uint16_t ymin, uint16_t ymax);
static int ram_set_cursor(uint16_t x, uint16_t y);
static int ram_write(uint8_t *bitmap);
static int ram_load(void);

/*** Interface Functions  ***/

/* Fully initialises device, starting SPI communications and setting
   the appropriate pins to the correct modes.

   Returns:
   0  Success.
   1  Fail, errno set to ECOMM */
int
epd_on(void)
{
    log_info("Initialising %s E Paper Device", DEVICE);

    if (init_gpio()) goto fail1; /* Initiates GPIO pins */
    if (init_spi())  goto fail1; /* Initiates SPI communications */

    epd_reset();		/* Wipes screen */

    if (push_shift_register())     goto fail1; /* Startup commands */
    if (push_lut(lut_full_update)) goto fail1; /* Sends look up table */

    /* RAM to hold full bitmap, representing pixels from origin to
       maximum dimensions */
    ram_set_window(0, WIDTH, 0, HEIGHT);

    return 0;

 fail1:
    log_err("Failed to initialise %s", DEVICE);
    errno = ECOMM;
    return 1;
}

/* Displays provided bitmap on epaper device display. Bitmap length
   must equal that of the display.

   Returns:
   0 Success.
   1 Fail, invalid bitmap length, errno set to EINVAL.
   2 Fail, SPI comms, errno set to ECOMM */
int
epd_display(uint8_t *bitmap, size_t len)
{
    log_info("Updating device display with %zu bitmap.", len);

    /* 1 byte can represent 8px across the width. However, an extra
       byte is required on each row if the number of px across the
       width is not a factor of 8.  */
    size_t width = (WIDTH % 8 == 0)
	? WIDTH / 8
	: WIDTH / 8 + 1; 

    if (len != width * HEIGHT)  goto fail1;

    if (ram_write(bitmap))      goto fail2;
    if (ram_load())             goto fail2;

    return 0;
 fail1:
    log_err("Invalid bitmap", DEVICE);
    errno = EINVAL;
    return 1;
 fail2:
    log_err("Failed to process bitmap on device (%s)", DEVICE);
    errno = ECOMM;
    return 2;
}

/* Resets the epaper display screen using the GPIO reset pin. */
void
epd_reset(void)
{
    log_info("Resetting device (%s) display", DEVICE);

    spi_gpio_write(PIN_RST, GPIO_LEVEL_HIGH);
    spi_delay(RESET_DELAY);
    spi_gpio_write(PIN_RST, GPIO_LEVEL_LOW);
    spi_delay(RESET_DELAY);
    spi_gpio_write(PIN_RST, GPIO_LEVEL_HIGH);
    spi_delay(RESET_DELAY);
    return;
}

/* Put device into deep sleep.

   Returns: 0  Success.
            1  Failed to enter deep sleep mode, errno set to EBUSY */
int
epd_off(void)
{
    log_info("Device (%s) entering deep sleep mode", DEVICE);

    uint8_t dsm[] = { 0x01 };

    if (wait_while_busy()) {
	log_err("Failed to sleep device");
	errno = EBUSY;
	return 1;
    }

    write_command(DEEP_SLEEP_MODE);
    write_data(dsm, ARRSIZE(dsm));

    return 0;
}

/* Returns width of epaper device screen in pixels */
uint16_t
epd_get_width(void)
{
    return WIDTH;
}

/* Returns height of epaper device screen in pixels */
uint16_t
epd_get_height(void)
{
    return HEIGHT;
}

/*** Static Functions ****/

/* Initialises GPIO pins and sets the correct GPIO operating
 * mode. Requirements documented in Waveshare manual page 9/26. */
static int
init_gpio(void)
{
    log_info("Initialising %s GPIO pins", DEVICE);

    if (spi_init_gpio()) {
	log_err("Failed to initialise %s GPIO pins", DEVICE);
	return 1;
    }
    
    spi_gpio_pinmode(PIN_RST,  SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_DC,   SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_CS,   SPI_PINMODE_OUTPUT);
    spi_gpio_pinmode(PIN_BUSY, SPI_PINMODE_INPUT);
    
    return 0;
}

/* Initiates SPI communication with epaper display unit.

   Returns:
   0 Success.
   1 Fail. */
static int
init_spi(void)
{
    log_info("Initialising %s SPI communication", DEVICE);

    if (spi_open(SPI_CHANNEL, SPI_CLK_HZ)) {
	log_err("Failed to initialise %s SPI communication", DEVICE);
	return 1;
    }

    return 0;
}

/* Sets the GPIO pins for command transfer and transfers given command
   to epaper device.

   Returns: 0 on success.
   1 on failure, sets errno to EIO */
static int
write_command(enum COMMAND command)
{
    /* DC pin low for command  */
    spi_gpio_write(PIN_DC, GPIO_LEVEL_LOW);
    spi_gpio_write(PIN_CS, GPIO_LEVEL_LOW);

    /* Ensure that command is on 8-bit byte with bitmask */
    uint8_t byte = command & 0xFF;

    if (spi_write(&byte, 1)) {
	log_err("Failed to send command byte to device %s", DEVICE);
	errno = EIO;
	return 1;
    }
    
    spi_gpio_write(PIN_CS, GPIO_LEVEL_HIGH);
    
    return 0;
}

/* Sets the GPIO pins for data transfer and transfers given
   data to epaper device.

   Returns: 0  on success.
   1  on failure, sets errno to EIO */
static int
write_data(uint8_t *data, size_t len)
{
    /* DC pin high for data transfer */
    spi_gpio_write(PIN_DC, GPIO_LEVEL_HIGH);
    spi_gpio_write(PIN_CS, GPIO_LEVEL_LOW);

    if (spi_write(data, len)) {
	log_err("Failed to send data to device %s", DEVICE);
	errno = EIO;
	return 1;
    }
    
    spi_gpio_write(PIN_CS, GPIO_LEVEL_HIGH);
    
    return 0;
}

/* Write the required data to the device shift register.

   Returns: 0, on success.
   1 SPI write error, sets errno to EIO . */
static int
push_shift_register(void) 
{
    /* Device specific data for commands in COMMAND enum, data
       variable name abbreviations correspond to COMMAND enum */
    uint8_t doc[]  = { (HEIGHT-1) & 0xFF, ((HEIGHT-1) >> 8) & 0xFF, 0x00 };
    uint8_t bssc[] = { 0xD7, 0xD6, 0x9D };
    uint8_t wvr[]  = { 0xA8 };
    uint8_t sdlp[] = { 0x1A };
    uint8_t sgt[]  = { 0x08 };
    uint8_t bwc[]  = { 0x03 };
    uint8_t dems[] = { 0x03 };

    if (write_command(DRIVER_OUTPUT_CONTROL))      goto fail1;
    if (write_data(doc, ARRSIZE(doc)))             goto fail1;
    if (write_command(BOOSTER_SOFT_START_CONTROL)) goto fail1;
    if (write_data(bssc, ARRSIZE(bssc)))           goto fail1;
    if (write_command(WRITE_VCOM_REGISTER))        goto fail1;
    if (write_data(wvr, ARRSIZE(wvr)))             goto fail1;
    if (write_command(SET_DUMMY_LINE_PERIOD))      goto fail1;
    if (write_data(sdlp, ARRSIZE(sdlp)))           goto fail1;
    if (write_command(SET_GATE_TIME))              goto fail1;
    if (write_data(sgt, ARRSIZE(sgt)))             goto fail1;
    if (write_command(BORDER_WAVEFORM_CONTROL))    goto fail1;
    if (write_data(bwc, ARRSIZE(bwc)))             goto fail1;
    if (write_command(DATA_ENTRY_MODE_SETTING))    goto fail1;
    if (write_data(dems, ARRSIZE(dems)))           goto fail1;

    return 0;
 fail1:
    log_err("Failed to write to device (%s) shift register", DEVICE);
    errno = EIO;
    return 1;
}

/* Send 30B look up table to device.

   Returns: 0 successful write.
   1 SPI write error, sets errno EIO. */
static int
push_lut(uint8_t *lut)
{
    if (write_command(WRITE_LUT_REGISTER)) goto fail1;
    if (write_data(lut, 30))               goto fail1;

    return 0;
 fail1:
    log_err("Failed to update device (%s) LUT", DEVICE);
    errno = EIO;
    return 1;
}
    
/* Pauses process until busy pin reads low or 100 x BUSY_DELAY
   seconds, which ever occurs first.
   Returns: 0 on success, device ready, GPIO busy pin is low.
   1 on error, device busy, sets errno to EBUSY */
static int
wait_while_busy(void)
{
    int t = 0;
    while (spi_gpio_read(PIN_BUSY) == GPIO_LEVEL_HIGH) {

	if (t > 100) {
	    errno = EBUSY;
	    log_err("Device (%s) not leaving busy state, "
		    "is power connected?.", DEVICE);
	    errno = EBUSY;
	    return 1;
	}
	 
	spi_delay(BUSY_DELAY);
	++t;
    }
	
    return 0;
}

/* Sets the RAM address start and end positions for the device. The range,
   and its corresponding area, determines the e-paper display
   window. Parameters should provided as pixel coordinates from origin of
   display.

   Returns: 0 Success.  
            1 Fail, errno set to ECOMM */
static int
ram_set_window(uint16_t xmin, uint16_t xmax,
	       uint16_t ymin, uint16_t ymax)
{
    /* One byte of data holds information for 8 pixels across the
       width. So 8px of data requires 1B of RAM, hence division by 8
       required (x >> 3) */
    // Does not account for sizes that are not factor of eight
    uint8_t x_start_end[] = { (xmin >> 3) & 0xFF, (xmax >> 3) & 0xFF };

    /* Pixel height can be greater than 255 (more than 1B) on this
       device. Masking required to send as two bytes. */
    uint8_t y_start_end[] = { ymin & 0xFF, (ymin >> 8) & 0xFF,
			      ymax & 0xFF, (ymax >> 8) & 0xFF };

    if (write_command(SET_RAM_X_ADDRESS_START_END_POSITION)) goto fail1;
    if (write_data(x_start_end, ARRSIZE(x_start_end)))       goto fail1;

    if (write_command(SET_RAM_Y_ADDRESS_START_END_POSITION)) goto fail1;
    if (write_data(y_start_end, ARRSIZE(y_start_end)))       goto fail1;

    return 0;
 fail1:
    log_err("Failed to set device (%s) display window", DEVICE);
    errno = ECOMM;
    return 1;
}
/* Sets the RAM cursor for next bitmap write. Takes coordinates that
   represent number of pixels from display origin.

   Returns: 0  Success.
            1  Fail, errno set to ECOMM. */
static int 
ram_set_cursor(uint16_t x, uint16_t y)
{
    /* One byte of data holds information for 8 pixels across the
       width. So 8px of data requires 1B of RAM, hence division by 8
       required (x >> 3) */
    uint8_t x_ram_start[] = { (x >> 3) & 0xFF };

    /* Pixel height can be greater than 255 (more than 1B) on this
       device. Masking required to send as two bytes. */
    uint8_t y_ram_start[] = { y & 0xFF, (y >> 8) & 0xFF };

    if (write_command(SET_RAM_X_ADDRESS_COUNTER))      goto fail1;
    if (write_data(x_ram_start, ARRSIZE(x_ram_start))) goto fail1; 

    if (write_command(SET_RAM_Y_ADDRESS_COUNTER))      goto fail1;
    if (write_data(y_ram_start, ARRSIZE(y_ram_start))) goto fail1;

    return 0;
 fail1:
    log_err("Failed to set device (%s) RAM cursor", DEVICE);
    errno = ECOMM;
    return 1;
}

/* Write the provided bitmap to the device RAM row by row.

   The RAM cursor is set before writing each row. The logical
   representation of black (0) and white (1) for this EPDM is the
   inverse of the PBM format supplied by bitmap.h. Consequently bytes
   must be inverted before they are written.

   Returns 0  Success.
           1  Fail, SPI comms failure, errno set to ECOMM. */
static int
ram_write(uint8_t *bitmap)
{
    /* Bitmap data must be passed row by row for this device. The
       cursor must be set at the start of each new row. */
    size_t row_bytes = (WIDTH % 8) ? 1 + (WIDTH / 8) : WIDTH / 8;

    for (size_t y = 0; y < HEIGHT; ++y) {
	if (ram_set_cursor(0, y))
	    goto fail1;

	if (write_command(WRITE_RAM))
	    goto fail1;
        
	for (size_t x = 0; x < row_bytes; ++bitmap, ++x) {

	    /* Logical representation of black and white pixels is
	       the inverse of that provided by bitmap.h */
	    uint8_t cur = ~(*bitmap);
	    if (write_data(&cur, 1))
		goto fail1;
	}
    }

    return 0;
 fail1:
    log_err("Failed to write to device (%s)", DEVICE);
    errno = ECOMM;
    return 1;
}

/* Load the bitmap stored in the RAM window, defined by
   ram_set_window() to the epaper device display.

   Returns: 0  Success.
            1  Fail, write error, errno set to ECOMM
            2  Fail, device busy, errno set to EBUSY */
static int
ram_load(void)
{
    uint8_t duc2[] = { 0xC4 };

    if (write_command(DISPLAY_UPDATE_CONTROL_2))   goto fail1;
    if (write_data(duc2, ARRSIZE(duc2)))           goto fail1;
    if (write_command(MASTER_ACTIVATION))          goto fail1;
    if (write_command(TERMINATE_FRAME_READ_WRITE)) goto fail1;

    if (wait_while_busy()) goto fail2;

    return 0;
 fail1:
    log_err("Failed to load device (%s) bitmap from RAM", DEVICE);
    errno = ECOMM;
    return 1;
 fail2:
    log_err("Failed to load device (%s) bitmap from RAM", DEVICE);
    errno = EBUSY;
    return 2;
}


