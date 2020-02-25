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

/* Description:

*/

#ifndef TEXT_H
#define TEXT_H

#include "bitmap.h"
#include "wordlist.h"
#include "oku_types.h"

struct TEXTBOX {
    FILE *text, *font;		/* Font and text files */
    unsigned fontsize;		/* Fontsize in pixels */
    WORDLIST *textbuf;		/* Unicode buffer */
    BITMAP *box;		/* Bitmap to render onto */
    coordinate x, y;		/* Cursors */
};

int textbox_init(struct TEXTBOX *txt);
int textbox_close(struct TEXTBOX *txt);
int textbox_write(struct TEXTBOX *txt);
int textbox_set_cursor(struct TEXTBOX *txt, coordinate x, coordinate y);

#endif	/* TEXT_H */
