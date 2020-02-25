/* text_freetype.c
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

#include "oku_types.h"
#include "text.h"

#define UNKNOWN_CHARACTER { 0x00, 0xEF, 0xBF, 0xBD };

FT_Library library;
FT_Face face;

/************************/
/* Forward Declarations */
/************************/

/* Static Functions */
static int face_load_glyph(byte *character[4]);
static resolution line_get_linespace(void);
static int line_is_row_full(coordinate cur_x, coordinate x_max, long advance);
static int line_is_height_full(coordinate cur_y, coordinate y_max,
			       long linespace);
static int line_write_char(struct BITMAP *bmp, coordinate x, coordinate y);
static int load_character(codepoint utf);

/* FreeType Wrappers */
static int freetype_error(int res);
static int freetype_library_init(void);
static int freetype_set_fontface(char *path);
static int freetype_set_fontsize(int fontsize);
static int freetype_set_charmap(FT_Encoding encoding);
static int freetype_get_char_index(unsigned long ch, int *out_idx);
static int freetype_load_glyph(int idx);
static int freetype_render_glyph(void);
static int freetype_free_face(void);
static int freetype_free_library(void);

/* Validation Functions */
static int check_valid_fontfile(char *fontfile);
static int check_valid_fontsize(char *fontsize);
static int check_valid_string(codepoint *string);
static int check_valid_bmp(struct BITMAP *bmp);
static int check_valid_coordinates(coordinate x, coordinate y);
static int check_glyph_rendered(unsigned char pixel_mode);
static int check_down_flow(int pitch);

/* Debugging */
static void debug_bitmap_to_stdout(void);
static int debug_bitmap_to_file(void);


/***********************/
/* Interface Functions */
/***********************/

/* Function: textbox_init()

   a textbox structure. The font file  */
int
textbox_initialise(struct TEXTBOX *txt)
{
 /*    if ( check_valid_txtbox() || */
 /* 	 freetype_library_init() ) */
 /* 	goto out1; */

 /*    if ( check_valid_fontfile(txt->fontfile) || */
 /* 	 freetype_set_fontface(txt->fontfile) ) */
 /* 	goto out2; */

 /*    if ( check_valid_fontsize(txt->fontsize) || */
 /* 	 freetype_set_fontsize(txt->font_size) ) */
 /* 	goto out2; */
	 
 /*    if ( freetype_set_charmap(FT_ENCODING_UNICODE) ) */
 /* 	goto out3; */
    
 /*    if ( check_valid_coordinates || */
 /* 	 textbox_set_cursor(txt, 0, 0) ) */
 /* 	goto out3; */

    
 /*    log_info("FreeType Initialised with:\n\t Font: %s.\n\t Size: %hu.\n\t", */
 /* 	     txt->font_file, txt->font_size); */

    return 0;
 /* out3: */
 /*    freetype_free_face(); */
 /* out2: */
 /*    freetype_free_library(); */
 /* out1: */
 /*    log_err("Cannot create textbox, invalid fields."); */
 /*    return 1; */
	
}

int
textbox_close(struct TEXTBOX *txt)
{
    /* if ( freetype_free_face() ) { */
    /* 	log_err("Failed to free FreeType font face memory."); */
    /* 	return 1; */
    /* } */

    /* if ( freetype_free_library() ) { */
    /* 	log_err("Failed to free FreeType library memory."); */
    /* 	return 1; */
    /* } */
    
    /*  txt->fontfile = NULL; */
    /*  txt->fontsize = 0; */
    /*  txt->string = NULL; */
    /*  txt->bmp = NULL; */
    /*  txt->x = 0; */
    /*  txt->y = 0; */

    /* log_info("Textbox closed."); */

    return 0;
}

int
textbox_write(struct TEXTBOX *txt)
{
    /* /\* Pixel counts *\/ */
    /* resolution dev_width  = txt->bmp->row_px; */
    /* resolution dev_height = txt->bmp->length / txt->bmp->pitch; */

    /* /\* Cursors *\/ */
    /* coordinate x = 0; */
    /* coordinate y = 0; */

    /* /\* Buffers for one utf character *\/ */
    /* byte character[4]      = { 0x00, 0x00, 0x00, 0x00 }; */
    /* byte character_prev[4] = { 0x00, 0x00, 0x00, 0x00 }; */
    
    /* while ( utf8_nextchar(txt->string, &character) == 0 ) { */

    /* 	// load character bitmap to face (global scope) */
    /* 	if ( char_load(&character) ) */
    /* 	    char_load_replacement(); */

    /* 	// determine character position */
    /* 	char_position(dev_width, dev_height, &x, &y); */

    /* 	// copy character */
    /* 	char_write(txt->bmp, &x, &y); */

    /* } */

    return 0;
}
 
int
textbox_set_cursor(struct TEXTBOX *txt, uint16_t x, uint16_t y)
{
    /* if (txt->bmp == NULL) { */
    /* 	log_err("Cannot set cursor, invalid bitmap."); */
    /* 	return 1; */
    /* } */

    /* size_t x_count = txt->bmp->row_px; */
    /* size_t y_count = txt->bmp->length / txt->bmp->pitch; */

    /* if ( x >= x_count || y >= y_count ) { */
    /* 	log_err("Cannot set cursor, invalid coordinates:\n\t" */
    /* 		"Given: (%lu, %lu) Max: (%zu, %zu).", */
    /* 		x, y, x_count - 1, y_count - 1); */
    /* 	return 2; */
    /* } */
	    
    /* txt->x = x; */
    /* txt->y = y; */

    return 0;
}
/********************/
/* Static Functions */
/********************/

/* Static function: face_load()

   Loads unicode 'character' into FreeType face (global scope).

   Returns:
   0 Success, character loaded to face.
   1 Failed to load character.
*/
static int
face_load_glyph(byte *character[4])
{
    return 0;
}

static resolution
line_get_linespace(void)
{
    return face->size->metrics.height / 64;
}

static int
line_is_row_full(coordinate cur_x, coordinate x_max, long advance)
{
    return x_max >= (cur_x + advance) ? 0 : 1;
}

static int
line_is_height_full(coordinate cur_y, coordinate y_max, long linespace)
{
    return y_max >= (cur_y + linespace) ? 0 : 1;
}

static int
line_write_char(struct BITMAP *bmp, coordinate x, coordinate y)
{
    struct BITMAP character;
    if ( bitmap_create(&character, face->glyph->bitmap.pitch * 8,
		       face->glyph->bitmap.rows) )
	return 1;

    character.buffer = face->glyph->bitmap.buffer;

    if ( bitmap_copy(bmp, &character, x, y) )
	return 2;

    return 0;
}

static int
load_character(codepoint utf)
{
    int ft_idx = 0;

    // get character string.

    if ( freetype_get_char_index(utf, &ft_idx) ) {
	log_err("Invalid character U+'%zx'.", utf);
	return 1;
    }

    if ( freetype_load_glyph(ft_idx) ) {
	log_err("Failed to load glyph:\n\t"
		"Character: '%zx', Index %d.", utf, ft_idx);
	return 1;
    }

    if ( freetype_render_glyph() ) {
	log_err("Failed to render glyph:\n\t"
		"Character: '%zx', Index %d.", utf, ft_idx);
	return 1;
    }

    if ( check_glyph_rendered(face->glyph->bitmap.pixel_mode) ||
	 check_down_flow(face->glyph->bitmap.pitch) ) {

	log_err("Glyph rendered incorrectly\n\t"
		"Character: '%zx', Index %d.", utf, ft_idx);
	return 1;
    }
    
    return 0;
}

/************************/
/* Validation Functions */
/************************/

/* Each function checks that the provided value is valid, returns 0 if
   valid and 1 if invalid. */

static int
check_valid_fontfile(char *fontfile)
{
    return 0;
}

static int
check_valid_fontsize(char *fontsize)
{
    return 0;
}

static int
check_valid_string(struct UTF *string)
{
    return 0;
}

static int
check_valid_bmp(struct BITMAP *bmp)
{
    return 0;
}

static int
check_valid_coordinates(coordinate x, coordinate y)
{
    return 0;
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

/******************************/
/* FreeType Wrapper Functions */
/******************************/

/* Static Function: freetype_error().

   Tests the FreeType return value. All freetype library functions
   that return error codes should be wrapped around this function.

   If FreeType library functions returns an error code, this function
   returns 1. Otherwise, 0 is returned.  */
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

/* Wrappers around FreeType functions that test the FreeType return
   value using Freetype_error(). Each function returns 0 on success or
   1 on failure. */

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
    int flags = FT_RENDER_MODE_MONO;
    return freetype_error( FT_Render_Glyph(face->glyph, flags) );
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

/***********************/
/* Debugging Functions */
/***********************/

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

