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

#include "text.h"
#include "oku_mem.h"
#include "oku_types.h"

/* Initialises FreeType library and sets the font face and size in
   pixels and returns handle. On error uninitialises and returns
   NULL. */
FT *
text_start(char *fontpath, unsigned fontsize)
{
    FT *ft = oku_alloc(sizeof *ft);
    FT_Init_FreeType(&ft->lib);
    FT_New_Face(ft->lib, fontpath, 0, &ft->face);
    FT_Set_Pixel_Sizes(ft->face, 0, fontsize);

    return ft;
}

/* Free all allocated memory within FT handle. */
void
text_stop(FT *delete)
{
    return;
}

/* Copies glyph from slot into out. */
static int
cptoglyph(FT *ft, codepoint unicode, FT_Glyph *out)
{
    unsigned index = FT_Get_Char_Index(ft->face, codepoint);
    FT_Load_Glyph(ft->face, index, FT_LOAD_DEFAULT);
    FT_Get_Glyph(face->glyph, out);

    return OK;
}

static int
measure_glyph(FT_Glyph glyph, FT_BBox *bbox)
{
    FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, bbox);
    
    return OK;
}

