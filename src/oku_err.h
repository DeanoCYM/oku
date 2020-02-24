/* oku_err.h
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

   Defines error number enumeration and provides interface for
   accessing descriptive strings.
*/

#ifndef OKU_ERR_H
#define OKU_ERR_H

enum OKU_ERRNO
    {
     /* No errors */
     OK                            = 0x00,
     /* Errors (Positive) */
     ERR_INPUT                     = 0x01,
     ERR_COMMS                     = 0x02,
     ERR_MEM                       = 0x03,
     ERR_IO                        = 0x04,
     ERR_UNINITIALISED             = 0x05,
     ERR_PARTIAL_WRITE   	   = 0x06,
     ERR_BUSY                      = 0x07,
     /* Warnings (Negative) */
     WARN_ROOT                     = -0x01
    };



#endif	/* OKU_ERR_H */
