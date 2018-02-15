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

#if ! defined(QUEUE_H)
#define QUEUE_H
#include <pthread.h>
	
#include "../error_logs/errno_logs.h"
#include "../data_structs/packets.h"
	
	struct queue_node {
		struct packet *data;
		struct queue_node *next;
	};
	
	typedef struct _queue {
		struct queue_node *head;
		struct queue_node *tail;
		
		pthread_mutex_t queue_lock;   //lock for queue objects
	} Generic_queue;

	int empty(Generic_queue *Q);

	Generic_queue *create_queue();

	struct packet *dequeue(Generic_queue *);

	void enqueue(Generic_queue *node, struct packet *data);
	
	void destroy_queue(Generic_queue *Q);
	
#endif
