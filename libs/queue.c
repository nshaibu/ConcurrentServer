#include <stdio.h>
#include <stdlib.h>

#include "queue.h"


Generic_queue *create_queue() {
	Generic_queue *node = (Generic_queue*)malloc(sizeof(Generic_queue));
	if (node == NULL)
		return NULL;
	
	node->head = NULL;
	node->tail = NULL;
	pthread_mutex_init( &(node->queue_lock), NULL);
	
	return node;
}

int empty(Generic_queue *Q) {
	return (Q->head == NULL);
}

void enqueue(Generic_queue *Q, struct packet *data) {
	struct queue_node* np = (struct queue_node*)malloc(sizeof(struct queue_node));
	np->data = data;
	np->next = NULL;
	
	if (empty(Q)) {
		Q->head = np;
		Q->tail = np;
	}
	else {
		Q->tail->next = np;
		Q->tail = np;
	}
	
	pthread_mutex_unlock( &(Q->queue_lock) );
}

struct packet *dequeue(Generic_queue *Q) {
	if ( empty(Q) ) 
		return NULL;
	
	struct packet *hold = Q->head->data;
	struct queue_node *temp = Q->head;
	
	Q->head = Q->head->next;
	if (Q->head == NULL) 
		Q->tail = NULL;
	
	free(temp);
	
	pthread_mutex_unlock( &(Q->queue_lock) );
	
	return hold;
} 

void destroy_queue(Generic_queue *Q) {
	struct packet *pk;
	
	while ( !empty(Q) ) {
		pthread_mutex_lock( &(Q->queue_lock) );   //lock queue mutex 
		pk = dequeue(Q);
		destroy_packet(pk);
	}
	
	pthread_mutex_destroy(&(Q->queue_lock));
}


#ifdef TRY
	#include <stdio.h>
	#include "queue.h"
	
	int main() {
		int n = 8;
		int temp = 41;
		int a = 10;
		int b = 58;
		
		Generic_queue *Q = create_queue();
		
		enqueue(Q, &n);
		enqueue(Q, &temp);
		enqueue(Q, &a);
		enqueue(Q, &b);
		
		while ( !empty(Q) ) {
			int *v = (int*)dequeue(Q);
			int c = *v;
			printf("%d\n", c);
		}
		printf("\n");
	}
	
#endif
