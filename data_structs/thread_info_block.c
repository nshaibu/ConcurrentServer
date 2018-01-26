#include <stdio.h>
#include <stdlib.h>

#include "thread_info_block.h"

time_t get_curr_time(void) { return  time(NULL); } //function pointer

struct thread_block *create_thread_node(unsigned tid, unsigned sockfd) 
{
	struct thread_block *node = (struct thread_block*)malloc(sizeof(struct thread_block));
	if (node == (struct thread_block*)NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	
	node->tid = tid;
	node->socket = sockfd;
	node->sub_tid = -1;
	node->start_time = time(NULL);
	node->curr_time = get_curr_time;
	node->node_op = THREAD_NOOP;
	
	node->loc.longitude = -1;
	node->loc.lattitude = -1;
	node->out_queue = create_queue();
	node->in_queue = create_queue();
	
	return node;
}

int destroy_thread_node(struct thread_block *node) {
	//code
}

void set_thread_node_socket(struct thread_block *th, int sockfd) {
	th->socket = sockfd;
}

void set_thread_node_tid(struct thread_block *th, int tid) {
	th->tid = tid;
}


void set_geolocation_info(struct thread_block *node, long _long, long _latti) {
	node->loc.longitude = ( _long <= 0 )? 0 : _long;
	node->loc.lattitude = (_long <= 0 )? 0 : _latti;
}

const struct geoloc *get_geolocation(struct thread_block *node) { return ( node == NULL ) ? NULL : &(node->loc); }

void set_outQueue(struct thread_block *blk, Generic_queue *Q) { if ( blk && Q ) { blk->out_queue = Q; } }
void set_inQueue(struct thread_block *blk, Generic_queue *Q) { if ( blk && Q ) { blk->in_queue = Q; } }

Generic_queue *get_outQueue(struct thread_block *blk) { return blk->out_queue; }
Generic_queue *get_inQueue(struct thread_block *blk) { return blk->in_queue; }
