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

#include <unistd.h>		/* read() */
#include <errno.h>		/* errno */

#include <wiringPi.h>
#include <wiringPiSPI.h>

#include "spi.h"
#include "oku_types.h"

/* Static storage and retrival of file descriptor number for use with
   write() */
static int spi_fid = -1;

/*** Interface  ***/

/* Initialises WiringPI SPI interface with Broadcom GPIO pin
   numbers. */
int
spi_init_gpio(void)
{
    wiringPiSetupGpio();	/* returns void but sets errno */
    switch (errno) {
    case 0:			/* No error */
	break;
    case EACCES:		/* not root (not critical) */
	errno = 0;
	return WARN_ROOT;
    default:			/* Critical error */
	return ERR_IO;
    }

    return OK;
}

/* Sets GPIO pin to provided mode. */
void
spi_gpio_pinmode(int pin, enum SPI_PINMODE mode)
{
    return pinMode(pin, mode);
}


/* Opens spi interface and stores the file descriptor in spi_fid. */
int
spi_open(int channel, int speed)
{
    return (spi_fid = wiringPiSPISetup(channel, speed)) < 0
	? ERR_COMMS : OK;
}

/* Set the value of the given GPIO pin  */
int
spi_gpio_write(int pin, enum GPIO_LEVEL pin_level)
{
    if ( pin_level == GPIO_LEVEL_ERROR )
	return ERR_COMMS;

    digitalWrite(pin, pin_level);

    return OK;

}

/* Get the value of a given GPIO pin

   Return values are cast to enum GPIO_LEVEL defined in spi_wp.h
   mimics the output from WiringPI: 0 GPIO low voltage; 1 GPIO high
   voltage; -1 Error. */
enum GPIO_LEVEL
spi_gpio_read(int pin)
{
    return digitalRead(pin);
}

/* Write n bytes to SPI interface. */
int
spi_write(byte *data, int len)
{
    if (spi_fid < 0)
	return ERR_UNINITIALISED;

    return write(spi_fid, data, len) < len ? ERR_PARTIAL_WRITE : OK;
}

/* Generic delay (guaranteed minimum delay time) */
void
spi_delay(unsigned int time)
{
    return delay(time);
}
