#ifndef THREAD_INFO_BLOCK_H
#define THREAD_INFO_BLOCK_H

#include <time.h>
#include <mysql.h>
#include <pthread.h>

#include "../libs/queue.h"
#include "../error_logs/errno_logs.h"

enum node_op_type {
	THREAD_IN,    /*Write operation*/
	THREAD_KILL,  /*Remove Node*/
	THREAD_OUT,   /*Read Operation*/
	THREAD_NOOP
};


//Geolocation struct
struct geoloc {
	long  longitude;
	long  lattitude;
};


struct thread_block {
	unsigned socket;       /* Thread's socket */
	pthread_t tid;    /* Main thread's id */
	pthread_t sub_tid; /* Sub-thread id for thread to poll on socket*/
	
	MYSQL *con;     /*mysql connection handler for the thread*/
	
	time_t start_time;    /* The started time */
	time_t (*curr_time)(void);    /* gets the current time*/
	
	struct geoloc loc;    /* geolocation information */
	
	/* The operation to be done on the thread_info_node:
		THREAD_IN: write from sendbox
		THREAD_OUT: read operation
		THREAD_KILL: Remove Node
		THREAD_NOOP: No Operation
	*/
	enum node_op_type node_op;
	Generic_queue *in_queue;
	Generic_queue *out_queue;
};

#define check_queue_init(Q) ( (Q) == NULL )

time_t get_curr_time(void);

//constructor and destructor
struct thread_block *create_thread_node(unsigned tid, unsigned sockfd);

int destroy_thread_node(struct thread_block *node);


//Operations:

void set_thread_node_socket(struct thread_block *th, int sockfd);

void set_thread_node_tid(struct thread_block *th, int tid);

void set_geolocation_info(struct thread_block *node, long _long, long _latti);

const struct geoloc *get_geolocation(struct thread_block *node);


//inbox and the sendbox operations
void set_outQueue(struct thread_block *, Generic_queue *);

void set_inQueue(struct thread_block *, Generic_queue *);

Generic_queue *get_outQueue(struct thread_block *);

Generic_queue *get_inQueue(struct thread_block *);

#endif
