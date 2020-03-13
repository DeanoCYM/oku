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

/***************/
/* Description */
/***************/

/* Glyph rendering interface. */

#ifndef GLYPH_H
#define GLYPH_H

#include "bitmap.h"
#include "oku_types.h"

typedef struct GLYPH {
    BITMAP *bmp;		/* Rendered gylph bitmap */
    resolution width;		/* Glyph width in px */
    resolution advance;		/* Distance from origin to start of next glyph */
    resolution baseline;	/* Distance from origin to baseline. */
} GLYPH;

/* Start and stop the rendering engine. Only one instance is permitted
   at one time. */
 int glyph_start_renderer(char *fontpath, unsigned fontsize);
void glyph_stop_renderer(void);

/* Returns a pointer to a glyph object, which contains a rendered
   character from provided unicode codepoint. */
GLYPH *glyph(codepoint unicode);
 void  glyph_delete(GLYPH *delete);

/* Returns linespace in px */
resolution glyph_linespace(void);


#endif	/* GLYPH_H */
