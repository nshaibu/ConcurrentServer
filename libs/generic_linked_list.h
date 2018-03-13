#ifndef GENERIC_LINKED_LIST
#define GENERIC_LINKED_LIST

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <pthread.h>

typedef enum {ENABLE_LOCK, DISABLE_LOCK} list_lock;

//The node for the list the void *data could 
//be allocated with malloc and does must be
//destroy in case anything goes wrong
struct list_node {
    void *data;    /*The data in the list node*/
    struct list_node *next, *prev;
};

typedef struct {
    struct list_node *head;
    struct list_node *tail;

    int list_len;                    /*The length of the linked list*/
    unsigned is_lock_used;           /*Determine whether the list was created to enable locking [ENABLE_LOCK|DISABLE_LOCK]*/
    pthread_mutex_t gen_list_lock;   /*mutex lock to ensure exclusive acess to list by all threads*/
} Generic_list;



#ifndef DTOR_IGN
void ignore(void *data);   //do nothing
#define DTOR_IGN ignore   //ignore destructor
#endif

Generic_list *create_generic_list(list_lock lock);

struct list_node *create_list_node(size_t size_alloc);

int list_lock_acquire(Generic_list *L);

int list_lock_release(Generic_list *L);

int list_lock_destroy(Generic_list *L);

int append_list_node(Generic_list *L, struct list_node *node);

int generic_list_empty(Generic_list *L);

void *list_popfront(Generic_list *);

void *list_popback(Generic_list *);

unsigned int list_pushback(Generic_list *L, void *data, void (*_dtor_)(void*));

unsigned int list_pushfront(Generic_list *L, void *data, void (*_dtor_)(void*));

int generic_list_insert_next_to(Generic_list *L, void *data, void(*_dtor_)(void*), struct list_node *next_to_node);

void destroy_list_node(Generic_list *L, struct list_node *node, void(*_dtor_)(void*));

void destroy_generic_list(Generic_list *L, void(*_dtor_)(void*));

#endif

