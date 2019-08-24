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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "thread_info_block.h"

time_t get_curr_time(void) { return  time(NULL); } //function pointer

struct thread_block *create_thread_node(struct list_node *tid, unsigned sockfd, Generic_list *list) 
{
	struct thread_block *node = (struct thread_block*)malloc(sizeof(struct thread_block));
	if (node == (struct thread_block*)NULL) {
		log_errors( NULL,
                  STD_ERRORS, 
                  DONT_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MEMORY_NOT_ALLOC, 
                  NULL, 
                  error_ignore);

		return NULL;
	}
	
	node->socket = sockfd;
	
	node->start_time = time(NULL);
	node->curr_time = get_curr_time;
	
	node->userid = -1;
	memset(node->username, '\0', sizeof(node->username));
	node->user_auth = USER_NOT_AUTH;
	node->revision_number = 0;

	node->con = mysql_init(NULL);
	if ( node->con == NULL ) {
		log_errors( NULL,
                  	MYSQL_ERRORS, 
                  	DONT_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	MYSQL_INIT_FAILED, 
                  	NULL,
                  	error_ignore );
		
		close(node->socket);
		
		free(node);
		return NULL;
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
		log_errors( NULL,
                  	MYSQL_ERRORS, 
                 	DONT_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	MYSQL_CONNECTION_FAILED, 
                  	NULL,
                  	error_ignore );
		
		mysql_close(node->con);
		close(node->socket);

		free(node);

		return NULL;
   }
	
	node->loc.longitude = -1;
	node->loc.lattitude = -1;
	
	node->in_queue = create_queue();
	if (node->in_queue == NULL) {
		errno = ENOMEM;
		log_errors( NULL,
                  STD_ERRORS, 
                  DONT_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  "Cannot initialize message queue", 
                  NULL, 
                  error_ignore);

		mysql_close(node->con);
		close(node->socket);
		free(node);

		return NULL;
	}

	node->tid = tid;
	node->tid_list = list;
	
	return node;
}

void destroy_thread_node(struct thread_block *node) {
	close(node->socket);
	mysql_close(node->con);

	node->user_auth = USER_TO_EXIT;  //kill connection handler thread

	list_lock_acquire( node->tid_list );
	SERVER_MSG("Detroying list node");
	destroy_list_node(node->tid_list, node->tid, free);
	list_lock_release( node->tid_list );

	destroy_queue(node->in_queue);
	
	free(node);
}

void set_thread_node_socket(struct thread_block *th, int sockfd) {
	th->socket = sockfd;
}

void set_thread_node_tid(struct thread_block *th, struct list_node *tid) {
	th->tid = tid;
}


void set_geolocation_info(struct thread_block *node, float _long, float _latti) {
	node->loc.longitude = _long;
	node->loc.lattitude = _latti;
}

const struct geoloc *get_geolocation(struct thread_block *node) { 
	return ( node == NULL ) ? NULL : &(node->loc); 
}


void set_inQueue(struct thread_block *blk, Generic_queue *Q) { 
	if ( blk && Q ) 
		blk->in_queue = Q; 
}


Generic_queue *get_inQueue(struct thread_block *blk) { 
	return blk->in_queue; 
}

void set_revision_num(struct thread_block *blk) {
	char db_query[MYSQL_QUERY_BUFF];
	MYSQL_RES *result;

	snprintf( db_query, MYSQL_QUERY_BUFF,
						"SELECT * FROM %s.users", 
						my_info.database_name );

	if ( mysql_query(blk->con, db_query) !=0 ) {
    	       		log_errors(  blk->tid,
        	                MYSQL_ERRORS, 
            	            DONT_EXIT, 
                	        NO_WRITE, 
                    	    LOGS_FATAL_ERRORS, 
                    	    MYSQL_QUERY_FAILED, 
                        	NULL, 
                        	error_ignore );
	}

	result = mysql_store_result(blk->con);
	int test = mysql_num_rows(result);

	blk->revision_number = test;
}
