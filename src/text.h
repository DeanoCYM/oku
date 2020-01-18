/* text.h
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

   Writes string to device.

*/

#include <stdint.h>
#include <sys/types.h>

int text_add_string(void);

/* Define the area to be used for writing text. Clears area setting
   pixels to white. Text origin set to (xmin, ymin).

   Returns:
   0 Success
   1 Error, coordinates out of bounds, errno set to EINVAL.
   2 Error, communitication error, errno set to EIO  */
//int text_set_area(uint16_t xmin, uint16_t xmax, uint16_t ymin, uint16_t ymax);

/* Sets the font (defaults to ...TBC).

   Returns:
   0 Success
   1 Error, font size not avaliable, errno set to EINVAL */
//int text_set_font(char *font);

/* Writes string to device. Text starts at origin of text area and
   proceeds until there is no remaining room.
   
   Returns:
   Number of characters written.
   -1 Communication error, errno set to EIO. */
// int text_write(const char *str, size_t len);

