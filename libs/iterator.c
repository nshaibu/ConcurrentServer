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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "iterator.h"

static unsigned list_len = 0;
static iterPtr head = NULL;
static iterPtr last = NULL;

static iterPtr forward_curr = NULL; //addr of the current iter traversed forward
static iterPtr backward_curr = NULL; //addr of the current iter traversed backward

iterPtr create_iterator(struct thread_block *block) {
    iterPtr node = (iterPtr)malloc(sizeof(iterator));
    if ( node == NULL ) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    node->info_block = block;
    node->_flag = NOT_BUSY;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

void link_iterator(iterPtr node) {
    if (list_len == 0) {
        head = last = node;
        node->next = head;
        forward_curr = backward_curr = head;
        ++list_len;

    } else {
        last->next = node;
        node->next = head;
        last = node;
        ++list_len;

    }
}

struct thread_block *remove_iterator(iterPtr node) {
    iterPtr prev = NULL;
    struct thread_block *hold = NULL;

    if (node == NULL) return NULL;

    if (node == head) {
        --list_len;
        head = node->next;
        last->next = head;

        hold = node->info_block;
        free(node);
    } else {
        prev = node;
        while ( prev->next != node ) prev = prev->next;
        prev->next = node->next;

        if ( node == last ) last = prev;
        hold = node->info_block;
        free(node);
        --list_len;
    }

    return hold;
}

iterPtr move_forward() {
    sleep(0.02);
    if ( forward_curr == NULL ) return NULL;
    else {
        forward_curr = forward_curr->next;
        return forward_curr;
    }
}

iterPtr move_backward() {
    if ( backward_curr == NULL ) return NULL;
    else {
        backward_curr = backward_curr->prev;
        return backward_curr;
    }
}

int get_iter_length() { return list_len; }

#ifdef TRY

int main(){
    int a = 12, b=21, c=45, d=78, e=14;
    iterPtr curr=NULL;
    iterPtr node1 = create_iterator(&a);
    link_iterator(node1);
    iterPtr node2 = create_iterator(&b);
    link_iterator(node2);

    iterPtr node3 = create_iterator(&c);
    link_iterator(node3);

    iterPtr node4 = create_iterator(&d);
    link_iterator(node4);

    iterPtr node5 = create_iterator(&e);
    link_iterator(node5);

    while ( (curr = move_forward()) ) {
        printf("%d:::%d\n", *curr->info_block, get_iter_length());
    }


}

#endif
