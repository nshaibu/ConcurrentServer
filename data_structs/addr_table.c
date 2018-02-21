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

#include "addr_table.h"

static int addrTable_len = 0;

static Addr_Table addrs[MAX_ADDR_TABLE_SIZE] = {0};

pthread_mutex_t addrTable_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int number_of_users = 0;   //global variable for the number of users on the system

int get_addr_table_len() { return addrTable_len; }

static int binary_search(int key) {
	int low = 0, hi = addrTable_len;
	if (key < 0) return -1;
	
	while ( low <= hi ) {
		int mid = ( low + hi )/2;

		if (addrs[mid].userid == key) return mid;
		if (addrs[mid].userid > key) hi = mid - 1;
		else low = mid + 1;
	}
	
	return -1;
}

void insertInAddrTable(unsigned id, void *addr) {
    int k = addrTable_len - 1;
    
    while (k >= 0 && k <= MAX_ADDR_TABLE_SIZE && addrs[k].userid > id ) {
        addrs[k+1] = addrs[k];
        --k;
    }

    addrs[k+1].userid = id;
    addrs[k+1].addr_ = addr;
    ++addrTable_len;
    
    pthread_mutex_unlock(&addrTable_mutex);
}

void *getAddrFromaddrTable(unsigned id) {
	int ret = binary_search(id);
	
	return ( ret >= 0 )? addrs[ret].addr_ : NULL;
}

void *removeFromAddrTable(unsigned id) {
	
	int pos = binary_search(id);
	if ( pos < 0 )
		return NULL;
	
	void *hold = addrs[pos].addr_;
	
	for (int i=pos; i<=addrTable_len; i++) {
		addrs[i] = addrs[i+1];
	}
	
	addrTable_len = addrTable_len - 1;
	return hold;
}

void destroy_addrTable() {
	
	if ( addrTable_len != 0 )
		for ( int i=addrTable_len-1; i>=0; --i ) {
			struct thread_block *node = (struct thread_block*)addrs[i].addr_;
			node->user_auth = USER_TO_EXIT;
		}
	
	pthread_mutex_unlock(&addrTable_mutex);
}



