/* spi_wp.c
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
 * Description:
 *
 * Wrapper around wiringPI package (http://wiringpi.com) implementing
 * spi.h interface. Provides SPI and GPIO communication between and
 * electronic paper display and controller.
 *
 */

#include "spi.h"

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <ert_log.h>
#include <wiringPi.h>
#include <wiringPiSPI.h>

#define BACKEND "WiringPi"

/* Static storage and retrival of file descriptor number for use with
   write() */
static int spi_fid = -1;

/*** Interface  ***/

/* Initialises WiringPI SPI interface with Broadcom GPIO pin numbers.

   Returns: 0 Success.
            1 Error, sets errno to EIO. */
int
spi_init_gpio(void)
{
    log_info("Initialising GPIO interface with %s", BACKEND);
    wiringPiSetupGpio();	/* returns void but sets errno */

    /* Process errno */
    switch (errno) {
    case EACCES:		/* not root */
	/* Not a critical error as many systems can communicate
	   without root privilages, error number should be reset even
	   if warning is complited out. */
	log_warn("GPIO operating %s without root privileges,\n\t"
		 "this may work dependant on hardware configuration.", BACKEND);
	if (LOGLEVEL < 1)				
	    errno = 0;

    case 0:			/* no error */
	break;
		
    default:			/* Critical error */
	log_err("Failed to initialise GPIO interface with %s", BACKEND);
	errno = EIO;
	return 1;
    }

    return 0;
}

/* Sets GPIO pin to provided mode.  */
void
spi_gpio_pinmode(int pin, enum SPI_PINMODE mode)
{
    log_debug("%s setting pin %d to %d", BACKEND, pin, mode);
    pinMode(pin, mode);
    return;
}

/* Opens spi interface and stores the file descriptor in spi_fid.

   Returns: 0  Success.
            1  Error, errno set to EIO */
int
spi_open(int channel, int speed)
{
    log_info("Opening SPI interface (channel %d %dHz) with %s backend",
	     channel, speed, BACKEND);

    int fid = wiringPiSPISetup(channel, speed);
    if (fid < 0) {
	log_err("Failed to set channel %d to %dHz with %s backend",
		  channel, speed, BACKEND);
	errno = EIO;
	return 1;
    }

    spi_fid = fid;		/* storage of spi_fid */
    return 0;
}

/* Set the value of the given GPIO pin  */
void
spi_gpio_write(int pin, int value)
{
    log_debug("Setting pin %02d to %02d with %s backend",
	     pin, value, BACKEND);
    digitalWrite(pin, value);
    return;
}

/* Get the value of a given GPIO pin

   Returns: 0  GPIO low voltage.
            1  GPIO high voltage.
	    -1 Error, errno set to EIO */
int
spi_gpio_read(int pin)
{
    return digitalRead(pin);
}

/* Write n bytes to SPI interface.

   Returns: 0, on success.
            1 Error, sets ERRNO to EIO or EBADF*/
int
spi_write(uint8_t *data, size_t len)
{
    if (spi_fid < 0) {
	log_err("Invalid file descriptor %d (%d)");
	errno = EBADF;
	return 1;
    }

    int res = write(spi_fid, data, len);

    /* Safe to compare signed and unsigned after checking that the
       signed is not negative. */
    if (res < 0 || (size_t) res < len) {
    	log_err("SPI write failure (%d)", res);
    	errno = EIO;
    	return 1;
    }

    return 0;

    /* for (size_t i = 0; i < len; ++i) { */
    /* 	res = wiringPiSPIDataRW(channel, data + i, 1); */
    /* 	if (res < 0) { */
    /* 	    log_err("SPI write error"); */
    /* 	    errno = EIO; */
    /* 	} */
    /* } */

    /* return (res < 0) ? 1 : 0; */

}
	      


/* Generic delay (guaranteed minimum delay time) */
void
spi_delay(unsigned int time)
{
    delay(time);
    return;
}
