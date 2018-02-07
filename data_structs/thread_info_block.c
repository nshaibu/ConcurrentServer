#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "thread_info_block.h"

time_t get_curr_time(void) { return  time(NULL); } //function pointer

struct thread_block *create_thread_node(unsigned tid, unsigned sockfd) 
{
	struct thread_block *node = (struct thread_block*)malloc(sizeof(struct thread_block));
	if (node == (struct thread_block*)NULL) {
		log_errors( NULL,
                  STD_ERRORS, 
                  DONT_EXIT, 
                  WRITE_STDDER, 
                  LOGS_WARNING, 
                  MEMORY_NOT_ALLOC, 
                  NULL, 
                  error_ignore);
	}
	
	node->tid = tid;
	node->socket = sockfd;
	
	node->start_time = time(NULL);
	node->curr_time = get_curr_time;
	
	node->node_op = THREAD_NOOP;
	
	node->userid = -1;
	memset(node->username, '\0', sizeof(node->username));
	node->user_auth = USER_NOT_AUTH;
	
	node->con = mysql_init(NULL);
	if ( node->con == NULL ) {
		log_errors( &(node->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_INIT_FAILED, 
                  node,
                  (void (*)(void*))destroy_thread_node);
   }

	if (! mysql_real_connect( node->con,
                             my_info.server_name,
                             my_info.user_name,
                             my_info.user_password,
                             NULL,
                             0, 
                             NULL,
                             0
									)
	) {
		log_errors( &(node->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_CONNECTION_FAILED, 
                  node,
                  (void (*)(void*))destroy_thread_node );
   }
	
	node->loc.longitude = -1;
	node->loc.lattitude = -1;
	//node->out_queue = create_queue();
	node->in_queue = create_queue();
	
	return node;
}

void destroy_thread_node(struct thread_block *node) {
	close(node->socket);
	mysql_close(node->con);
	
	destroy_queue(node->in_queue);
	
	free(node);
}

void set_thread_node_socket(struct thread_block *th, int sockfd) {
	th->socket = sockfd;
}

void set_thread_node_tid(struct thread_block *th, int tid) {
	th->tid = tid;
}


void set_geolocation_info(struct thread_block *node, long _long, long _latti) {
	node->loc.longitude = ( _long < 0 )? -1 : _long;
	node->loc.lattitude = (_long < 0 )? -1 : _latti;
}

const struct geoloc *get_geolocation(struct thread_block *node) { 
	return ( node == NULL ) ? NULL : &(node->loc); 
}

/*void set_outQueue(struct thread_block *blk, Generic_queue *Q) { */
/*	if ( blk && Q ) */
/*		blk->out_queue = Q;  */
/*}*/

void set_inQueue(struct thread_block *blk, Generic_queue *Q) { 
	if ( blk && Q ) 
		blk->in_queue = Q; 
}

/*Generic_queue *get_outQueue(struct thread_block *blk) { */
/*	return blk->out_queue; */
/*}*/

Generic_queue *get_inQueue(struct thread_block *blk) { 
	return blk->in_queue; 
}
