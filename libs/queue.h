#if ! defined(QUEUE_H)
#define QUEUE_H
	
	#include "../data_structs/packets.h"
	
	struct queue_node {
		struct packet *data;
		struct queue_node *next;
	};
	
	typedef struct _queue {
		struct queue_node *head, *tail;
	} Generic_queue;

	int empty(Generic_queue *Q);

	Generic_queue *create_queue();

	struct packet *dequeue(Generic_queue *);

	void enqueue(Generic_queue *node, struct packet *data);
	
	void destroy_queue(Generic_queue *Q);
	
#endif
