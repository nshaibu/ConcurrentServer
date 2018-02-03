#ifndef THREAD_INFO_BLOCK_H
#define THREAD_INFO_BLOCK_H

#include <time.h>
#include <mysql.h>
#include <pthread.h>

#include "../libs/queue.h"
#include "../error_logs/errno_logs.h"
#include "../server.h"

#if !defined ( NAME_BUF_SIZE ) || ! defined ( PASSWD_BUF_SIZE ) || ! defined ( MSG_BUFF_SIZE )
#define NAME_BUF_SIZE 100
#define PASSWD_BUF_SIZE 120
#define MSG_BUFF_SIZE 1000
#endif

enum node_op_type {
	THREAD_IN,    /*Write operation*/
	THREAD_KILL,  /*Remove Node*/
	THREAD_OUT,   /*Read Operation*/
	THREAD_NOOP
};

typedef enum { 
	USER_NOT_AUTH,     /*user not authenticated yet*/
	USER_AUTH,         /*user authenticated*/
	USER_TO_EXIT       /*authenticated user exit*/
} user_auth_level;

//Geolocation struct
struct geoloc {
	long  longitude;
	long  lattitude;
};


struct thread_block {
	unsigned socket;       /* Thread's socket */
	pthread_t tid;    /* thread's id */
	
	/*user data*/
	int userid;      /*authenticated userid for this thread*/
	char username[NAME_BUF_SIZE]; /*authenticated username for this thread*/
	user_auth_level user_auth;     /*Determine whether the user has been authenticated or exit loop(listening polling)*/
	
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
	//Generic_queue *out_queue;
};

#define check_queue_init(Q) ( (Q) == NULL )

time_t get_curr_time(void);

//constructor and destructor
struct thread_block *create_thread_node(unsigned tid, unsigned sockfd);

void destroy_thread_node(struct thread_block *node);


//Operations:

void set_thread_node_socket(struct thread_block *th, int sockfd);

void set_thread_node_tid(struct thread_block *th, int tid);

void set_geolocation_info(struct thread_block *node, long _long, long _latti);

const struct geoloc *get_geolocation(struct thread_block *node);


//inbox and the sendbox operations
//void set_outQueue(struct thread_block *, Generic_queue *);

void set_inQueue(struct thread_block *, Generic_queue *);

Generic_queue *get_outQueue(struct thread_block *);

Generic_queue *get_inQueue(struct thread_block *);

#endif
