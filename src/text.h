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

/* Renders text to bitmap surface using unicode codepoints. */

#ifndef TEXT_H
#define TEXT_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "oku_types.h"

/* Node to cache a glyph  */
typedef struct GLYPH {
    unsigned      index;	/* FreeType glyph index */
    unsigned long unicode;	/* Unicode codepoint */
    FT_Glyph      glyph;	/* FreeType glyph */
    FT_BBox       bbox;		/* Glyph bounding box */
} GLYPH;

typedef struct TEXT {
    FT_Library lib;		/* FreeType library handle */
    FT_Face    face;		/* Font face handle */
    GLYPH      db[STRMAX];	/* Cache for storing glyphs */
} TEXT;

/* Initialise FreeType*/
FT *text_start(char *font, unsigned size);




#endif	/* TEXT_H */

