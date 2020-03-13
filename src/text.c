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

/***************/
/* Description */
/***************/

/* Implements glyph rendering using FreeType. */

#include <unistd.h>		/* access() */
#include <assert.h>

#include "glyph.h"
#include "bitmap.h"
#include "cache.h"
#include "oku_types.h"
#include "oku_mem.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_MAX 50		/* maximum font size */

/* FreeType Objects */
FT_Library library = NULL;
FT_Face face = NULL;

/* Cached characters table */
CACHE *db = NULL;

/*************************/
/* Forward Decelerations */
/*************************/

/* Glyph allocation and destruction */
static GLYPH *create_glyph(codepoint unicode);
static  void  destroy_glyph(GLYPH *delete);

/* Glyph retrival */
static GLYPH *glyph_from_face(codepoint unicode);
static GLYPH *glyph_from_cache(codepoint unicode);

/* FreeType face object (global) routines */
static        int render(codepoint unicode);

/* Validation */
static int check_glyph_rendered(void);
static int check_down_flow(void);

/* FreeType Wrappers */
static  int ft_error(int res);
static  int ft_library_init(void);
static  int ft_set_fontface(char *path);
static  int ft_set_fontsize(int fontsize);
static  int ft_set_charmap(FT_Encoding encoding);
static  int ft_get_char_index(unsigned long ch, int *out);
static  int ft_load_glyph(int idx);
static  int ft_render_glyph(void);
static void ft_free_face(void);
static void ft_free_library(void);

/*************/
/* Interface */
/*************/

/* Frees all memory in database and FreeType. */
void
glyph_stop_renderer(void)
{
    ft_free_face();
    ft_free_library();
    cache_destroy(db);

    return;
}

/* Initialises FreeType library and sets the font and size. */
int
glyph_start_renderer(char *fontpath, unsigned fontsize)
{
    int err = OK;
    err = ft_library_init();
    if (err > 0) goto out;

    err = ft_set_fontface(fontpath);
    if (err > 0) goto out;
    err = ft_set_fontsize(fontsize);
    if (err > 0) goto out;
    err = ft_set_charmap(FT_ENCODING_UNICODE);
    if (err > 0) goto out;

    db = cache_create(fontpath, fontsize);

 out:
    return (err > 0 || db == NULL) ? glyph_stop_renderer(), err : OK;
}

/* Returns a pointer to a rendered glyph object at the codepoint
   unicode.

   First searches for a cached glyph, if this is not avaliable a new
   glyph is generated. Returns NULL on failure */
GLYPH *
glyph(codepoint unicode)
{
    GLYPH *new = glyph_from_cache(unicode);
    return new ? new : glyph_from_face(unicode);
}

void
glyph_delete(GLYPH *delete)
{
    return destroy_glyph(delete);
}

/* Provides the linespace in pixels for the current font face. */
resolution
glyph_linespace(void)
{
    return face == NULL ? ERR_UNINITIALISED : face->size->metrics.height / 64;
}

/********************/
/* Static Functions */
/********************/

/* Returns a new glyph object from the face and frees the FreeType
   face as it is no longer required. */
static GLYPH *
create_glyph(codepoint unicode)
{
    /* Compute dimensions of new glyph and create. */
    GLYPH *new    = oku_alloc(sizeof *new);
    new->advance  = face->glyph->advance.x / 64;
    new->baseline = face->glyph->bitmap_top;
    new->width    = face->glyph->bitmap_left;
    new->bmp      = bitmap_create(face->glyph->bitmap.width,
				  face->glyph->bitmap.rows);

    int err = bitmap_clear(new->bmp);
    if (err > 0)
	goto out;

    /* Bitmap object is manually created, rather than with
       bitmap_create(). This is so that a pointer to the FreeType
       generated bitmap can be used. This allows safe copying using
       bitmap_copy(). */
    BITMAP tmp;
    tmp.buffer = face->glyph->bitmap.buffer;
    tmp.pitch  = face->glyph->bitmap.pitch;
    tmp.width  = face->glyph->bitmap.width;
    tmp.length = tmp.pitch * face->glyph->bitmap.rows;

    /* Copy temporary bitmap into new glyph */
    err = bitmap_copy(new->bmp, &tmp, 0, 0);
    if (err > 0)
	goto out;

    /* Insert into cache. */
    err = cache_insert(db, unicode, new);

 out:
    return err > 0 ? NULL : new;
}

static void
destroy_glyph(GLYPH *delete)
{
    if (delete == NULL)
	return;

    bitmap_destroy(delete->bmp);
    delete->bmp      = NULL;
    delete->width    = 0;
    delete->advance  = 0;
    delete->baseline = 0;

    return oku_free(delete);
}
	

/* Retrieve glyph from cache or return NULL if not found. */
static GLYPH *
glyph_from_cache(codepoint unicode)
{
    return cache_search(db, unicode);
}

/* Render a new glyph.

   If there is an error rendering the glyph, return NULL. Otherwise
   use the rendering to create a new glyph and return this. */
static GLYPH *
glyph_from_face(codepoint unicode)
{
    return render(unicode) ? NULL : create_glyph(unicode);
}

static int
render(codepoint unicode)
{
    int err = OK;
    int idx = 0;		/* FreeType character index */

    /* Render */
    err = ft_get_char_index(unicode, &idx);
    if (err) goto out;
    err = ft_load_glyph(idx);
    if (err) goto out;
    err = ft_render_glyph();
    if (err) goto out;

    /* Validate rendering */
    err = check_glyph_rendered();
    if (err) goto out;
    err = check_down_flow();
    
 out:
    return err;
}

/********************/
/* Input Validation */
/********************/

static int
check_glyph_rendered(void)
{
    return face->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_MONO
	? ERR_RENDER : OK;
}

static int
check_down_flow(void)
{
    return face->glyph->bitmap.pitch < 0 ? ERR_RENDER : OK;
}

/******************************/
/* FreeType Wrapper Functions */
/******************************/

/* Static Function: ft_error().

   Tests the FreeType return value and converts them into the error
   states defined int oku_types.h.

   All freetype library functions that return error codes should be
   wrapped around this function.

   If FreeType library functions returns an error code, this function
   returns non zero. Warnings are < 0, errors > 0. On success 0 is
   returned.  */
static int
ft_error(int res)
{
    switch ( res ) {

    case FT_Err_Ok:
	res = OK;
	break;

    default:
	res = ERR_RENDER;
    }
    
    return res;
}

/*********************/
/* FreeType Wrappers */
/*********************/

/* Wrappers around FreeType functions that test the FreeType return
   value using Ft_error(). Each function returns OK, positive on
   a failure or negative with a warning. */

static int
ft_library_init(void)
{
    return ft_error( FT_Init_FreeType(&library) );
}

/* First check that the file can be read, then set font in FreeType
   library returning any error codes.  */
static int
ft_set_fontface(char *path)
{
    return access(path, R_OK) == -1
	? ERR_IO : ft_error( FT_New_Face(library, path, 0, &face) );
}

/* Firt ensure that the font size is between 0 and 50 set fontsize in
   FreeType library returning any error codes. */
static int
ft_set_fontsize(int fontsize)
{
    return (fontsize == 0 || fontsize > FONT_MAX)
	? ERR_INPUT : ft_error( FT_Set_Pixel_Sizes(face, fontsize, 0) );
}

static int
ft_set_charmap(FT_Encoding encoding)
{
    return ft_error( FT_Select_Charmap(face, encoding) );
}

static int
ft_get_char_index(unsigned long ch, int *out)
{
    return ( *out = FT_Get_Char_Index(face, ch) ) == 0
	? ERR_RENDER : OK;
}

static int
ft_load_glyph(int idx)
{
    int flags = FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
    return ft_error( FT_Load_Glyph(face, idx, flags) );
}

static int
ft_render_glyph(void)
{
    int flags = FT_RENDER_MODE_MONO;
    return ft_error( FT_Render_Glyph(face->glyph, flags) );
}

static void
ft_free_face(void)
{
    FT_Done_Face(face);
    face = NULL;
    return;
}

static void
ft_free_library(void)
{
    FT_Done_FreeType(library);
    library = NULL;
    return;
}

/***********************/
/* Debugging Functions */
/***********************/

//static void
//debug_bitmap_to_stdout(void)
//{
//    unsigned int pitch = (unsigned int)face->glyph->bitmap.pitch;
//    unsigned int height = face->glyph->bitmap.rows;
//    uint8_t *bitmap = face->glyph->bitmap.buffer;
//
//    for ( unsigned int row = 0; row < height; ++row ) {
//	printf("Row %02u: 0x ", row);
//	for ( size_t byte = 0; byte < pitch; ++byte )
//	    printf("%02hhx ", bitmap[byte + (row * pitch)]);
//	printf("\n");
//    }
//
//    return;
//}
//
//static int
//debug_bitmap_to_file(void)
//{
//    char file[] = "./char.pbm";
//    unsigned int height = face->glyph->bitmap.rows;
//    unsigned int pitch = (unsigned int)face->glyph->bitmap.pitch;
//    size_t len = (size_t)pitch * height;		   
//    uint8_t *bitmap = face->glyph->bitmap.buffer;
//
//    log_debug("Printing freetype bitmap to %s.", file);
//
//    /* Create new pbm file */
//    FILE *PBM = fopen(file, "w");
//    if ( !PBM ) {
//    	log_err("Failed to open file %s.", file);
//    	return 1;
//    }
//
//    /* Write headers */
//    fprintf(PBM, "P4 %d %d\n", pitch*8, height);
//    log_info("PBM headers: P4 %d %d\n", pitch*8, height);
//
//    /* Write data */
//    log_info("Writing %luB of data in %luB chunks to %s.",
//    	     len, sizeof bitmap[0], file);
//
//    size_t written = 0;
//    written = fwrite(bitmap, sizeof bitmap[0], len, PBM);
//    if ( written < len ) {
//    	log_err("Incomplete write (%lu/%lu) to %s.", written, len, file);
//    	return 1;
//    }
//
//    if ( fclose(PBM) )
//	log_warn("Failed to close file %s.", file);
//
//    return 0;
//}
//
//
