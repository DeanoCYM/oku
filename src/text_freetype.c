/* text.c
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

/* Description: */

#include <stdio.h>
#include <stdint.h>
#include <ert_log.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "epd.h"
#include "text.h"
#include "bitmap.h" 		/* remove after debugging */


FT_Library library;
FT_Face face;

/************************/
/* Forward Declarations */
/************************/
static int freetype_library_init(void);
static int freetype_set_fontface(char *path);
static int freetype_set_fontsize(int fontsize);
static int freetype_set_charmap(FT_Encoding encoding);
static int freetype_get_char_index(unsigned long ch, int *out_idx);
static int freetype_load_glyph(int idx);
static int freetype_render_glyph(void);
static int freetype_free_face(void);
static int freetype_free_library(void);
static int check_glyph_rendered(unsigned char pixel_mode);
static int check_down_flow(int pitch);
static int check_valid_bitmap(FT_Bitmap *bitmap);
static int freetype_error(int res);
static void debug_bitmap_to_stdout(void);
static int debug_bitmap_to_file(void);

/***********************/
/* Interface Functions */
/***********************/

int
textbox_init(struct TEXTBOX *txt)
{
    if ( freetype_library_init() ) {
	log_err("Failed to initialise FreeType");
	goto out1;
    }

    if ( freetype_set_fontface(txt->font_file) ) {
	log_err("Failed to open font file:\n\t%s", txt->font_file);
	goto out2;
    }

    if ( freetype_set_fontsize(txt->font_size) ) {
	log_err("Invalid font size %d", txt->font_size);
	goto out3;
    }

    if ( freetype_set_charmap(FT_ENCODING_UNICODE) ) {
	log_err("Invalid character map");
	goto out3;
    }
    
    log_info("FreeType Initialised with:\n\t Font: %s.\n\t Size: %d.",
	     txt->font_file, txt->font_size);

    return 0;
 out3:
    freetype_free_face();
 out2:
    freetype_free_library();
 out1:
    return 1;
	
}

int
textbox_close(struct TEXTBOX *txt)
{
    if ( freetype_free_face() ) {
	log_err("Failed to free FreeType font face memory.");
	return 1;
    }

    if ( freetype_free_library() ) {
	log_err("Failed to free FreeType library memory.");
	return 1;
    }
    
    txt->font_file = NULL;
    txt->font_size = 0;

    log_info("Textbox closed.");

    return 0;
}

int
textbox_write(struct TEXTBOX *txt, struct BITMAP *bmp)
{
    unsigned long ch = 'g';
    int idx = 0;

    if ( freetype_get_char_index(ch, &idx) ) {
	log_err("Invalid character '%c'.", 0xFF & ch);
	return 1;
    }

    if ( freetype_load_glyph(idx) ) {
	log_err("Failed to load glyph:\n\tCharacter: '%c', Index %d.",
		0xFF & ch, idx);
	return 1;
    }

    if ( freetype_render_glyph() ) {
	log_err("Failed to render glyph:\n\tCharacter: '%c', Index %d.",
		0xFF & ch, idx);
	return 1;
    }
	 
    debug_bitmap_to_stdout();
    debug_bitmap_to_file();

    struct BITMAP res;
    if ( bitmap_create(&res, face->glyph->bitmap.pitch * 8,
		       face->glyph->bitmap.rows) )
	return 1;
    res.buffer = face->glyph->bitmap.buffer;
    if ( bitmap_copy(bmp, &res, 0, 0))
	return 1;
    
    return 0;
}

/********************/
/* Static Functions */
/********************/

static int
freetype_library_init(void)
{
    return freetype_error( FT_Init_FreeType(&library) );
}

static int
freetype_set_fontface(char *path)
{
    return freetype_error( FT_New_Face(library, path, 0, &face) );
}

static int
freetype_set_fontsize(int fontsize)
{
    return freetype_error( FT_Set_Pixel_Sizes(face, fontsize, 0) );
}

static int
freetype_set_charmap(FT_Encoding encoding)
{
    return freetype_error( FT_Select_Charmap(face, encoding) );
}

static int
freetype_get_char_index(unsigned long ch, int *out_idx)
{
    return ( *out_idx = FT_Get_Char_Index(face, ch) ) == 0 ? 1 : 0;
}

static int
freetype_load_glyph(int idx)
{
    int flags = FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
    return freetype_error( FT_Load_Glyph(face, idx, flags) );
}

static int
freetype_render_glyph(void)
{
    return freetype_error( FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO) );
}

static int
freetype_free_face(void)
{
    return freetype_error( FT_Done_Face(face) );
}

static int
freetype_free_library(void)
{
    return freetype_error( FT_Done_FreeType(library) );
}

static int
check_glyph_rendered(unsigned char pixel_mode)
{
    return pixel_mode == FT_PIXEL_MODE_MONO ? 0 : 1;
}

static int
check_down_flow(int pitch)
{
    return pitch < 0 ? 1 : 0;
}

static int
check_valid_bitmap(FT_Bitmap *bitmap)
{
    return bitmap == NULL ? 1 : 0;
}

static int
freetype_error(int res)
{
    switch ( res ) {

    case FT_Err_Ok:
	break;

    default:
	log_err("FreeType error %d.", res);
	return 1;
    }
    
    return 0;
}

static void
debug_bitmap_to_stdout(void)
{
    unsigned int pitch = (unsigned int)face->glyph->bitmap.pitch;
    unsigned int height = face->glyph->bitmap.rows;
    uint8_t *bitmap = face->glyph->bitmap.buffer;

    for ( unsigned int row = 0; row < height; ++row ) {
	printf("Row %02u: 0x ", row);
	for ( size_t byte = 0; byte < pitch; ++byte )
	    printf("%02hhx ", bitmap[byte + (row * pitch)]);
	printf("\n");
    }

    return;
}

static int
debug_bitmap_to_file(void)
{
    char file[] = "./char.pbm";
    unsigned int height = face->glyph->bitmap.rows;
    unsigned int pitch = (unsigned int)face->glyph->bitmap.pitch;
    size_t len = (size_t)pitch * height;		   
    uint8_t *bitmap = face->glyph->bitmap.buffer;

    log_debug("Printing freetype bitmap to %s.", file);

    /* Create new pbm file */
    FILE *PBM = fopen(file, "w");
    if ( !PBM ) {
    	log_err("Failed to open file %s.", file);
    	return 1;
    }

    /* Write headers */
    fprintf(PBM, "P4 %d %d\n", pitch*8, height);
    log_info("PBM headers: P4 %d %d\n", pitch*8, height);

    /* Write data */
    log_info("Writing %luB of data in %luB chunks to %s.",
    	     len, sizeof bitmap[0], file);

    size_t written = 0;
    written = fwrite(bitmap, sizeof bitmap[0], len, PBM);
    if ( written < len ) {
    	log_err("Incomplete write (%lu/%lu) to %s.", written, len, file);
    	return 1;
    }

    if ( fclose(PBM) )
	log_warn("Failed to close file %s.", file);

    return 0;
}
