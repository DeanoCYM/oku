/* cpqueue.c
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

/* Unicode Codepoint Queue.

   FIFO linked list for holding 32bit unicode codepoint data. */

#include "bitmap.h"
#include "oku_types.h"

typedef struct CP_NODE {
    codepoint unicode;		/* 32bit unicode codepoint */
    BITMAP *rendering;		/* Rendered gylph bitmap */
    resolution advance;		/* Advance width in px */
    coordinate x, y;		/* Start pixel coordinates */
    struct CP_NODE *next;		/* Next node in list */
} CP_NODE;

typedef struct CP_QUEUE {
    unsigned long count;
    struct CP_NODE *head, *tail;
} CP_QUEUE;
    
/* Queue allocation and deallocation */
CP_QUEUE *cpq_create(void);
int cpq_destroy(CP_QUEUE *cpq); /* deallocates queue and all nodes */

/* Queue interface */
int cpq_enqueue(CP_QUEUE *cpq, codepoint unicode); /* add a node */
int cpq_dequeue(CP_QUEUE *cpq, CP_NODE **out);	   /* detach a node */
int cpq_delete(CP_NODE *delete);		   /* free a node */
