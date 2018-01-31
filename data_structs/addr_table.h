#ifndef ADDR_TABLE_H
#define ADDR_TABLE_H

#include <stdlib.h>
#include <pthread.h>

#include "thread_info_block.h"

typedef struct _addr {
    unsigned userid;  //The userid
    void *addr_;       //The address of the iterator user by the userid
} Addr_Table;

#define MAX_ADDR_TABLE_SIZE 100

void insertInAddrTable(unsigned id, void *addr);

void *getAddrFromaddrTable(unsigned id);

void *removeFromAddrTable(unsigned id);

int get_addr_table_len();

void destroy_addrTable();

#endif
