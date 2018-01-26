#if ! defined(QUEUE_H)
#define QUEUE_H
	
	struct queue_node {
		void *data;
		struct queue_node *next;
	};
	
	typedef struct _queue {
		struct queue_node *head, *tail;
	} Generic_queue;

	int empty(Generic_queue *Q);

	Generic_queue *create_queue();

	void *dequeue(Generic_queue *);

	void enqueue(Generic_queue *node, void *data);
	
#endif
