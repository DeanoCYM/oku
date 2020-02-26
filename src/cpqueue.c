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

#include "cpqueue.h"
#include "oku_types.h"
#include "oku_mem.h"

/* Allocates memory for a new queue */
CP_QUEUE *
cpq_create(void)
{
    CP_QUEUE *out = oku_alloc(sizeof *out);

    out->count = 0;
    out->head = NULL;
    out->tail = NULL;

    return out;
}

/* Free all memory associated with queue and nodes. */
int cpq_destroy(CP_QUEUE *cpq)
{
    int err = OK;
    CP_NODE *detached_node;

    while ( cpq->count > 0 ) {

	/* side effect: cpq_dequeue() decrements count */
	err = cpq_dequeue(cpq, &detached_node);
	if (err) break;

	err = cpq_delete(detached_node);
	if (err) break;
    }
    
    oku_free(cpq);

    return err;
}

/* Append a node to the end of the queue */
int cpq_enqueue(CP_QUEUE *cpq, codepoint unicode)
{
    CP_NODE *new = oku_alloc(sizeof *new);
    new->unicode = unicode;
    new->next = NULL;

    if (cpq->count == 0) {
	cpq->head = new;
	cpq->tail = new;
    } else {
	cpq->tail->next = new;
	cpq->tail = new;
    }

    ++cpq->count;
    
    return OK;
}

/* Detach a node from the queue */
int cpq_dequeue(CP_QUEUE *cpq, CP_NODE **out)
{
    if ( cpq->count == 0)
	return WARN_MTBUFFER;

    *out = cpq->head;
    cpq->head = cpq->head->next;

    if (--cpq->count == 0)
	cpq->tail == NULL;

    return OK;
}

/* Free the memory associated with a node */
int cpq_delete(CP_NODE *delete)
{
    if (delete == NULL)
	return ERR_UNINITIALISED;

    oku_free(delete);

    return OK;
}
