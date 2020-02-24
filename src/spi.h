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

#include "oku_types.h"

/* Operating modes from GPIO pins. */
enum SPI_PINMODE
    { SPI_PINMODE_INPUT, SPI_PINMODE_OUTPUT,
      SPI_PINMODE_PWM,   SPI_PINMODE_CLOCK };

/* GPIO levels, dictated by return value of digitalRead() in
   wiringPI library. */
enum GPIO_LEVEL
    { GPIO_LEVEL_ERROR = -1,
      GPIO_LEVEL_LOW   =  0,
      GPIO_LEVEL_HIGH  =  1 };

/*************/
/* Interface */
/*************/

/* All interface functions with int type return values return error
   codes defined in oku_err.h */

/* Initialises WiringPI SPI interface with Broadcom GPIO pin numbers. */
int spi_init_gpio(void);

/* Sets GPIO pin to 'mode' */
void spi_gpio_pinmode(int pin, enum SPI_PINMODE mode);

/* Write the logic level of a given GPIO pin to the provided value */
int spi_gpio_write(int pin, enum GPIO_LEVEL pin_level);

/* Read the logic level of a given GPIO pin */
enum GPIO_LEVEL spi_gpio_read(int pin);

/* Open spi interface. */
int spi_open(int channel, int speed);

/* Write len bytes to SPI interface. */
int spi_write(byte *data, int len);

/* Generic delay (guaranteed minimum delay time) */
void spi_delay(unsigned int time);

#endif	/* SPI_H */
