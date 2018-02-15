/*===========================================================================================
# Copyright (C) 2018 Nafiu Shaibu.
# Purpose: 
#-------------------------------------------------------------------------------------------
# This is a free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This is distributed in the hopes that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#===========================================================================================
*/

#ifndef ITERATOR_H
#define ITERATOR_H

#include "../data_structs/thread_info_block.h"

enum busy_flags {NOT_BUSY, BUSY};

typedef struct iter {
    struct thread_block *info_block;    //The data of the thread_info_block

    /*This flag checks whether another handler is
     * on this iterator*/
    enum busy_flags  _flag;

    struct iter *next, *prev;
} iterator, *iterPtr;

iterPtr create_iterator(struct thread_block *);

void link_iterator(iterPtr);

struct thread_block *remove_iterator(iterPtr);

int get_iter_length();

iterPtr move_forward();

iterPtr move_backward();

#endif
