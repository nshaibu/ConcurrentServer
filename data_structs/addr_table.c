#include "addr_table.h"

static int addrTable_len = 0;
static Addr_Table addrs[MAX_ADDR_TABLE_SIZE] = {0};

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
}

void *getAddrFromaddrTable(unsigned id) {
	int ret = binary_search(id);
	
	return ( ret > 0 )? addrs[ret].addr_ : NULL;
}

void *removeFromAddrTable(unsigned id) {
	
	int pos = binary_search(id);
	if (ret < 0)
		return NULL;
	
	void *hold = addrs[pos].addr_;
	
	for (int i=pos; i<=addrTable_len; i++) {
		addrs[i] = addrs[i+1]
	}
	
	addrTable_len = addrTable_len - 1;
	return hold;
}


#ifdef TRY
#include <stdio.h>

int main() {
    int a = 2;
    insertInAddrTable(2, &a);
    int b = 4;
    insertInAddrTable(1, &b);
    int c = 47;
    insertInAddrTable(41, &c);
    int d = 12;
    insertInAddrTable(12, &d);
    int e = 45;
    insertInAddrTable(23, &e);
    
    int *v = (int*)getAddrFromaddrTable(1);
    printf("%d\n", *v);
}
#endif
