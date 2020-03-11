/* cache.h
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

/***************/
/* Description */
/***************/

#ifndef CACHE_H
#define CACHE_H

/* Character bitmap cache

   A hash table to store rendered character glyph bitmaps. */

#include "glyph.h"
#include "oku_types.h"

typedef struct CACHE CACHE;

/* Allocates memory for, and returns a pointer to, a new glyph
   cache. Returns NULL if the fontname is invalid or the fontsize is
   zero. */
CACHE *cache_create(char *fontname, unsigned fontsize);

/* Destroy all memory associated with a glyph cahce */
void cache_destroy(CACHE *delete);

GLYPH *cache_search(CACHE *in, codepoint search);
int cache_insert(CACHE *in, codepoint unicode, GLYPH *new);

#endif	/* CACHE_H */


