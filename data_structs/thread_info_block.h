/*===========================================================================================
# Copyright (C) 2018 Nafiu Shaibu.
# Purpose: 
#-------------------------------------------------------------------------------------------
# This is a free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This is distributed in the hopes that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
         
#===========================================================================================
*/

#ifndef THREAD_INFO_BLOCK_H
#define THREAD_INFO_BLOCK_H

#include <time.h>
#include <mysql.h>
#include <pthread.h>

#include "../libs/queue.h"
#include "../error_logs/errno_logs.h"
#include "../libs/generic_linked_list.h"
#include "../server.h"

#if !defined ( NAME_BUF_SIZE ) || ! defined ( PASSWD_BUF_SIZE ) || ! defined ( MSG_BUFF_SIZE )
#define NAME_BUF_SIZE 100
#define PASSWD_BUF_SIZE 120
#define MSG_BUFF_SIZE 1000
#endif


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
	struct list_node *tid;    /* thread's id list node in generic list intialize in server.c[makeserver] */
	Generic_list *tid_list;   /*List of thread ids*/

	/*user data*/
	int userid;      /*authenticated userid for this thread*/
	char username[NAME_BUF_SIZE]; /*authenticated username for this thread*/
	user_auth_level user_auth;     /*Determine whether the user has been authenticated or exit loop(listening polling)*/
	
	MYSQL *con;     /*mysql connection handler for the thread*/
	
	time_t start_time;    /* The started time */
	time_t (*curr_time)(void);    /* gets the current time*/
	
	struct geoloc loc;    /* geolocation information */
	
	/*message queue for client. messages are stored here 
	temporary before client reads*/
	Generic_queue *in_queue;
};

#define check_queue_init(Q) ( (Q) == NULL )

time_t get_curr_time(void);

//constructor and destructor
struct thread_block *create_thread_node(struct list_node *tid, unsigned sockfd, Generic_list *list);

void destroy_thread_node(struct thread_block *node);


//Operations:

void set_thread_node_socket(struct thread_block *th, int sockfd);

void set_thread_node_tid(struct thread_block *th, struct list_node *tid);

void set_geolocation_info(struct thread_block *node, long _long, long _latti);

const struct geoloc *get_geolocation(struct thread_block *node);



void set_inQueue(struct thread_block *, Generic_queue *);

Generic_queue *get_outQueue(struct thread_block *);

Generic_queue *get_inQueue(struct thread_block *);

#endif
