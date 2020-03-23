/* 
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

/* Converts a unicode codepoint into a FreeType glyph. */

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

/* #include <stdio.h>  */   // debugging to stdout
/* #include <stdlib.h> */     

#include "text.h"
#include "bitmap.h"
#include "oku_mem.h"
#include "oku_types.h"

/* Initialise FreeType library and return handle */
FT *
text_start(char *font, unsigned size)
{
    TEXT *new = oku_alloc(sizeof *new);

    int err =
	FT_Init_FreeType(&new->lib)             ||
	FT_New_Face(ft->lib, font, 0 &ft->face) ||
	FT_Set_Pixel_Sizes(ft->face, 0, size);	

    

    return err ? ERR_RENDER : OK;
}

void
text_stop(FT *delete)
{
    FT_Done_Face(delete->face);
    FT_Done_FreeType(delete->lib);

    return;
}

