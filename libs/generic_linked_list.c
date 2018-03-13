#include "generic_linked_list.h"

void ignore(void *data) {}

Generic_list *create_generic_list(list_lock lock) 
{
    Generic_list *np = (Generic_list*)malloc( sizeof(Generic_list) );
    if ( np == NULL )
        return NULL;

    np->head = NULL;
    np->tail = NULL;
    np->list_len = 0;

    np->is_lock_used = lock;

    if (np->is_lock_used == ENABLE_LOCK)
        pthread_mutex_init(&(np->gen_list_lock), NULL);

    return np;
}

int list_lock_acquire(Generic_list *L) 
{
    return (L->is_lock_used == ENABLE_LOCK)? pthread_mutex_lock( &(L->gen_list_lock) ): -1;
}

int list_lock_release(Generic_list *L) 
{
    return (L->is_lock_used == ENABLE_LOCK)? pthread_mutex_unlock( &(L->gen_list_lock) ): -1;
}

int list_lock_destroy(Generic_list *L) 
{
    return (L->is_lock_used == ENABLE_LOCK)? pthread_mutex_destroy( &(L->gen_list_lock) ): -1;
}

void destroy_generic_list(Generic_list *L, void(*_dtor_)(void*))
{
    if ( L != NULL ) {
        if (L->head != NULL) {
            while (L->list_len !=0 )
            {
                void *data = list_popfront(L);
                _dtor_(data);
            }
        }
        list_lock_release(L);
        list_lock_destroy(L);
    } 
        
}

struct list_node *create_list_node(size_t size_alloc)
{
    struct list_node *np = (struct list_node*)malloc(sizeof(struct list_node));
    if (np == NULL) 
        return NULL;

    np->data = malloc(size_alloc);
    if (np->data == NULL) 
    {
        free(np);
        return NULL;
    }

    np->next = NULL;
    np->prev = NULL;

    return np;
}

int append_list_node(Generic_list *L, struct list_node *node) 
{
    if (node == NULL)
        return -1;

    if ( generic_list_empty(L) ) {
        L->head = L->tail = node;
        L->list_len++;
    } else {
        L->tail->next = node;
        node->prev = L->tail;
        L->tail = node;
        L->list_len++;
    }

    return 0;
}

int generic_list_empty(Generic_list *L) {
    return L->head == NULL; 
}

/*Insert a new node next to another node (next_to_node)*/
int generic_list_insert_next_to(Generic_list *L, void *data, void(*_dtor_)(void*), struct list_node *next_to_node) 
{
    unsigned test = 0;
    if (data == NULL)
        goto error_EINVAL;
    
    if (next_to_node == NULL) list_pushfront(L, data, _dtor_);
    else {
        struct list_node *np = (struct list_node*)malloc(sizeof(struct list_node));
        if (np == NULL)
            goto error_ENOMEM;
        
        np->data = data;
        np->next = next_to_node->next;
        np->prev = next_to_node;
        next_to_node->next->prev = np;   //next node point back to new node
        next_to_node->next = np;

        L->list_len++;

        if (next_to_node == L->tail)
            L->tail = np;
    }
    test = 1;

    error_ENOMEM:
        _dtor_(data);

    error_EINVAL:
        return test;
}

/*This function push items to the end of the list. In 
case you want to destroy data pass destructor function pointer 
to it and also if you want the exit status of the function pass an integer 
variable to the function or NULL if not.*/
unsigned list_pushback(Generic_list *L, void *data, void (*_dtor_)(void*) ) 
{
    unsigned test = 0;

    struct list_node *np = (struct list_node*)malloc(sizeof(struct list_node));
    if (np == NULL) goto error_ENOMEM;    //memory was not allocated
    else {
	    np->data = data;
	    np->next = NULL;
    }
	
	if ( generic_list_empty(L) ) {
		np->prev = NULL;
        L->head = np;
		L->tail = np;
        L->list_len++;
	}
	else {
        np->prev = L->tail;
		L->tail->next = np;
		L->tail = np;
        L->list_len++;
	}
    
    test = 1;
    
    error_ENOMEM:
        if (test != 1) 
            _dtor_(data);
        return test;  
}

/*It pushes an item in front of the list. SAME:prototype as above*/
unsigned list_pushfront(Generic_list *L, void *data, void (*_dtor_)(void*) ) 
{
    unsigned test = 0;
    struct list_node *np = (struct list_node*)malloc(sizeof(struct list_node));
    if (np == NULL)
        goto error_ENOMEM;
    else {
	    np->data = data;
	    np->prev = NULL;
    }
    
	if ( generic_list_empty(L) ) {
        np->next = NULL;
        L->head = np;
		L->tail = np;
        L->list_len++;
	}
	else {
		np->next = L->head;
        L->head->prev = np;
        L->head = np;
        L->list_len++;
	}

    test = 1;

    error_ENOMEM:
        if (test != 1) 
            _dtor_(data);
        return test;

}

void *list_popfront(Generic_list *L) 
{
 	if ( generic_list_empty(L) ) 
		return NULL;
	
	void *hold = L->head->data;
	struct list_node *temp = L->head;
	
	L->head = L->head->next;
	if (L->head == NULL) 
		L->tail = NULL;
    else
        L->head->prev = NULL;

    L->list_len--;
	
	free(temp);
	
	return hold;   
}

void *list_popback(Generic_list *L) 
{
    if ( generic_list_empty(L) ) 
	    return NULL;

    void *hold = L->tail->data;
    struct list_node *temp = L->tail;

    L->tail = L->tail->prev;
    if (L->tail == NULL)
        L->head = NULL;
    else
        L->tail->next = NULL;

    L->list_len--;

    free(temp);
    return hold;
} 


/*remove and destroy a node from the list. if the _dtor_ is not NULL then the data in
lis_node datastructure is destroy by using the specified function pointer*/ 
void destroy_list_node(Generic_list *L,  struct list_node *node, void (*_dtor_)(void*)) 
{
    if (L == NULL && node == NULL) goto DESTROY_NODE;

    if ( (node->next == NULL && node->prev == NULL) && node != L->head )  //Then this node has not been
        goto DESTROY_NODE;                                                //inserted in the list yet 

    if ( L->head != NULL ) {
        if (node == L->head){ 
            L->head = node->next;
            if (L->head == NULL)
                L->tail = NULL;
        } else {
            if (node == L->tail)
                L->tail = node->prev;

            node->prev->next = node->next;  //previous node points to next node after "node"
            if (node->next != NULL) node->next->prev = node->prev;  //node after "node" points back to previous node   
        }

        L->list_len--;
    }

    DESTROY_NODE:
        if ( node != NULL ) {
            if (node->data != NULL) _dtor_(node->data);
            free(node);
        }
}


#ifdef _TRY

int main() {
    Generic_list *L = create_generic_list(DISABLE_LOCK);
    if (L == NULL) return 1;
    int a = 6, b = 10;
    list_pushback(L, &a, DTOR_IGN);
    list_pushfront(L, &b, DTOR_IGN);
    int c = 20, d=45, e=12, f=30, g=85, h=78, i=852;
    list_pushfront(L, &c, DTOR_IGN);
    list_pushfront(L, &d, DTOR_IGN);
    list_pushfront(L, &e, DTOR_IGN);
    list_pushfront(L, &f, DTOR_IGN);
    list_pushfront(L, &g, DTOR_IGN);
    list_pushfront(L, &h, DTOR_IGN);
    generic_list_insert_next_to(L, &i, DTOR_IGN, L->head);

    destroy_list_node(L, L->head->next->next, DTOR_IGN);

    while (L->list_len > 0)
        printf("%d\n", *(int*)list_popback(L));
    return 0;
}

#endif