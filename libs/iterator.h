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
