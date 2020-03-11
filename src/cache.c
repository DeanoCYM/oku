/* cache.c
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

/* Character bitmap cache

   A hash table to store rendered character glyph bitmaps. */

#include "cache.h"
#include "glyph.h"

#include "oku_types.h"
#include "oku_mem.h"

#define TABLE_SIZE 255		/* Total number of Lists in table */

struct NODE {
    codepoint unicode;
    struct GLYPH *render;
    unsigned long hits;
    struct NODE *next;
};

struct LIST {
    struct NODE *head;
    struct NODE *tail;
};

struct CACHE {
    struct LIST table[TABLE_SIZE];
    char *fontname;
    unsigned fontsize;
};

/************************/
/* Forward Declarations */
/************************/

/* Cache */
static byte hash (codepoint unicode);

/* List */
static  void  ldelete_head (struct LIST *delete);
static  void  ldestroy     (struct LIST *delete);
static   int  lenqueue     (struct LIST *in, codepoint unicode, GLYPH *new);
static GLYPH *lsearch      (struct LIST *in, codepoint search);

/* Node */
static struct NODE *ncreate (codepoint unicode, GLYPH *newglyph);
static        void  ndelete (struct NODE *delete);
static       GLYPH *nsearch (struct NODE *head, codepoint search);

/* Validation */
static int nullptr(void *ptr);
static int cpsame(codepoint cp1, codepoint cp2);
/*******************/
/* Cache Interface */
/*******************/

/* Allocates memory for the cache structure (an array of lists).

   Ensures that the font arguements are valid and starts the font
   rendering backend. */
CACHE *
cache_create(char *fontname, unsigned fontsize)
{
    CACHE *new = oku_alloc(sizeof *new);
    new->fontname = fontname;
    new->fontsize = fontsize;

    for (int i = 0; i < TABLE_SIZE; ++i) {
	new->table[i].head = NULL;
	new->table[i].tail = NULL;
    }
	
    return new;
}

/* Free all memory in the cache and stop rendering backend. */
void
cache_destroy(CACHE *delete)
{
    for (int i = 0; i < TABLE_SIZE; ++i)
	ldestroy(&delete->table[i]);

    delete->fontname = NULL;
    delete->fontsize = 0;

    return;
}

/* Search for a node containing a codepoint in the cache and return
   pointer to its glyph.

   If the codepoint is not present in the table, or if the provided
   cache is NULL, NULL is returned. */
GLYPH *
cache_search(CACHE *in, codepoint search)
{
    return nullptr(in) ? NULL : lsearch(&in->table[hash(search)], search);
}

int
cache_insert(CACHE *in, codepoint unicode, GLYPH *new)
{
    return lenqueue(&in->table[hash(unicode)], unicode, new);
}

/********************/
/* Static Functions */
/********************/

static byte
hash(codepoint unicode)
{
    return unicode && 0xFF;
}

/********/
/* List */
/********/

/* Remove the head node, replacing it with the next node. The
   replacement node will be NULL if the list becomes empty. */
static void
ldelete_head(struct LIST *delete)
{
    if (nullptr(delete->head))
	return;

    struct NODE *new_head = delete->head->next;
    ndelete(delete->head);
    delete->head = new_head;

    /* When list is empty tail should also point to NULL. */
    if (nullptr(delete->head))
	delete->tail = NULL;
    
    return;
}

static void
ldestroy(struct LIST *delete)
{
    while (!nullptr(delete))	
	ldelete_head(delete);

    return;
}

/* Insert a new node containing new glyph into the list. */
static int
lenqueue(struct LIST *in, codepoint unicode, GLYPH *new)
{
    struct NODE *newtail = ncreate(unicode, new);
    in->tail->next = newtail;
    in->tail = newtail;

    return OK;
}
/* Attempts to find codepoint search in a list of nodes.

   If found, returns a pointer to the glyph contained within matching
   node. Otherwise returns NULL.*/
static GLYPH *
lsearch(struct LIST *in, codepoint search)
{
    return nsearch(in->head, search);
}

/********/
/* Node */
/********/

/* Allocate memory and initialise new node */
static struct NODE *
ncreate(codepoint unicode, GLYPH *newglyph)
{
    struct NODE *newnode = oku_alloc(sizeof *newnode);

    newnode->unicode = unicode;
    newnode->render  = newglyph;
    newnode->hits    = 0;
    newnode->next    = NULL;

    return newnode;
}

/* Free all memory associated with a node including the glyph. */
static void
ndelete(struct NODE *delete)
{
    if (nullptr(delete))
	return;

    delete->unicode = 0;
    delete->hits = 0;
    delete->next = NULL;

    glyph_delete(delete->render);
    
    return;
}

/* Move through the linked list checikng if search matches the unicode
   codepoint. If found, return a pointer to the nodes GLYPH character
   object. Otherwise return NULL.  */
static GLYPH *
nsearch(struct NODE *head, codepoint search)
{
    while ( !nullptr(head) && !cpsame(head->unicode, search) )
	head = head->next;

    return nullptr(head) ? NULL : head->render;
}
    
/**************/
/* Validation */
/**************/

/* Returns OK (0) if not NULL. */
static int
nullptr(void *ptr)
{
    return ptr == NULL ? ERR_UNINITIALISED : OK;
}

/* Returns OK (0) on match */
static int
cpsame(codepoint cp1, codepoint cp2)
{
    return cp1 != cp2 ? ERR_NOT_FOUND : OK;
}
    
