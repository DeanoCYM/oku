/* spi.h
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
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
 * Interface for SPI and GPIO communication between display device and
 * controller.
 * 
 */

#ifndef SPI_H
#define SPI_H

#include "wiringPi.h"

#include <sys/types.h>
#include <stdint.h>

enum SPI_PINMODE
    { SPI_PINMODE_INPUT  = INPUT,
      SPI_PINMODE_OUTPUT = OUTPUT,
      SPI_PINMODE_PWM    = PWM_OUTPUT,
      SPI_PINMODE_CLOCK  = GPIO_CLOCK  };

/* Initialises WiringPI SPI interface with Broadcom GPIO pin numbers.
   Returns: 0 Success.
            1 Error, sets errno to EIO. */
int spi_init_gpio(void);

/* Sets GPIO pin to 'mode' */
void spi_gpio_pinmode(int pin, enum SPI_PINMODE mode);

/* Write the logic level of a given GPIO pin to the provided value */
void spi_gpio_write(int pin, int value);

/* Read the logic level of a given GPIO pin
   Returns: 0  GPIO low voltage.
            1  GPIO high voltage.
	    -1 Error, errno set to EIO */
int spi_gpio_read(int pin);

/* Open spi interface.
   Returns: 0  Success.
            1  Error, sets errno to EIO. */
int spi_open(int channel, int speed);

/* Write n bytes to SPI interface.
   Returns: 0  Success.
            1  SPI Error, errno set to EIO */
int spi_write(const uint8_t *data, size_t len);

/* Generic delay (guaranteed minimum delay time) */
void spi_delay(unsigned int time);

#endif	/* SPI_H */
