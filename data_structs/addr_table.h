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

#ifndef ADDR_TABLE_H
#define ADDR_TABLE_H

#include <stdlib.h>
#include <pthread.h>

#include "thread_info_block.h"

extern pthread_mutex_t addrTable_mutex;

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
