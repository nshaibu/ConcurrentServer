#include "connect_handler.h"

static inline void send_msg_dontwait(struct thread_block *blk, const char *msg){
	send(blk->socket, msg, strlen(msg), MSG_DONTWAIT);
}


static int registrator(struct thread_block *blk, const char *name, const char *passwd){
	char db_query[MYSQL_QUERY_BUFF];
	MYSQL_RES *mysql_res = NULL;
	
	sprintf( db_query,
			 "SELECT userid FROM %s.users WHERE username = '%s' AND userpasswd = '%s'",
			 my_info.database_name, name, passwd);
			 
	if ( mysql_query(blk->con, db_query) !=0 ) {
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  NO_WRITE, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_QUERY_FAILED, 
                  blk, 
                  (void (*)(void*))destroy_thread_node );
	}
	
	mysql_res = mysql_store_result(blk->con);
	int test = mysql_num_rows(mysql_res);   //test whether user is unique
	
	if ( test == 0 ) {
		sprintf( db_query, 
					"INSERT INTO %s.users(username, userpasswd) VALUES ( '%s', '%s')",
					my_info.database_name, name, passwd);
					
		if ( mysql_query(blk->con, db_query) !=0 ) {
			log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  NO_WRITE, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_QUERY_FAILED, 
                  blk, 
                  (void (*)(void*))destroy_thread_node );
		} else 
			return 0;
	}
	
	return -1;
}


static int authenticator(struct thread_block *blk, const char name[], const char passwd[]) {
	char db_query[MYSQL_QUERY_BUFF];
	MYSQL_RES *mysql_res = NULL;
	MYSQL_ROW mysql_row;
	
	sprintf( db_query, 
			   "SELECT userid, username FROM %s.users WHERE username = '%s' AND userpasswd = '%s'",
			   my_info.database_name, name, passwd);
	
	if ( mysql_query(blk->con, db_query) !=0 ) {
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  NO_WRITE, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_QUERY_FAILED, 
                  blk, 
                  (void (*)(void*))destroy_thread_node );
	}
	
	mysql_res = mysql_use_result(blk->con);
	if (mysql_res == NULL) {
		mysql_free_result(mysql_res);
		
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  NO_WRITE, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_RESULT_CANT_READ, 
                  blk, 
                  (void (*)(void*))destroy_thread_node );	
	}
	
	mysql_row = mysql_fetch_row(mysql_res);   //get username and id in array
	if ( mysql_row == NULL ) {
		mysql_free_result(mysql_res);
		
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DONT_EXIT, 
                  NO_WRITE, 
                  LOGS_INFO, 
                  MYSQL_NO_USER, 
                  NULL, 
                  error_ignore );	
		return -1;   //no such user
	}
	
	char tmp[10];
	
	unsigned long *len = mysql_fetch_lengths(mysql_res);
	
	bcopy( mysql_row[0], tmp, len[0] );    //set thread block user id 
	blk->userid = atoi(tmp);
		
	struct thread_block *node = getAddrFromaddrTable(blk->userid);
	if ( node != NULL ) { //if user has already log in return
		mysql_free_result(mysql_res);
		return -1;
	}
	
	bcopy( mysql_row[1], &(blk->username), len[1] );   //set thread block user name
		
	mysql_free_result(mysql_res);
		
	pthread_mutex_unlock(&addrTable_mutex);
	insertInAddrTable(blk->userid, (void*)blk);   //put userid and thread_block address in address table
	
	blk->user_auth = USER_AUTH;   //set that user has been authenticated
	
	return 0;
}


static void authenticate_user(struct thread_block *blk, struct packet *pk) {
	char passwd[PASSWD_BUF_SIZE], name[NAME_BUF_SIZE], str[MAX_PACKET_SIZE];
	char *saveptr, *pword = NULL, *nword = NULL;
	
	strcpy(str, pk->msg);
	destroy_packet(pk);
	
	nword = strtok_r(str, ":", &saveptr);
	pword = strtok_r(NULL, ":", &saveptr);
	
	if ( nword != NULL && pword != NULL ) {
		strcpy(name, nword);
		strcpy(passwd, pword);
		
		int ret = authenticator(blk, name, passwd);
		
		if ( ret == 0 ) {
			sprintf(str, "|%d|-1|-1|-1|ACK|", ACK_PACKET);
			send_msg_dontwait(blk, str);
		} else {    //authentication failed
			sprintf(str, "|%d|-1|-1|-1|FIN|", FIN_PACKET);
			send_msg_dontwait(blk, str);
			
			//destroy_thread_node(blk);
			blk->user_auth = USER_TO_EXIT;
			pthread_cancel( pthread_self() );
		}
	} else {    //if any word is null exit
		sprintf(str, "|%d|-1|-1|-1|FIN|", FIN_PACKET);  /*send fin packet and close*/

		send_msg_dontwait(blk, str);											  /*connects*/
		//destroy_thread_node(blk);
		blk->user_auth = USER_TO_EXIT;
		pthread_cancel( pthread_self() );
	}
}



static void register_new_user(struct thread_block *blk, struct packet *pk) {
	char passwd[PASSWD_BUF_SIZE], name[NAME_BUF_SIZE], str[MAX_PACKET_SIZE];
	char *saveptr, *pword = NULL, *nword = NULL;
	
	//May have email address
	
	strcpy(str, pk->msg);
	destroy_packet(pk);
	
	nword = strtok_r(str, ":", &saveptr);
	pword = strtok_r(NULL, ":", &saveptr);
	
	if ( nword != NULL && pword != NULL ) {
		strncpy(name, nword, NAME_BUF_SIZE);
		strncpy(passwd, pword, PASSWD_BUF_SIZE);
		
		int ret = registrator(blk, name, passwd);
		if ( ret == 0 ) {
			sprintf(str, "|%d|-1|-1|-1|ACK|", ACK_PACKET );
			send_msg_dontwait(blk, str);
		} else {    //authentication failed
			sprintf(str, "|%d|-1|-1|-1|FIN|", FIN_PACKET);
			send_msg_dontwait(blk, str);

			destroy_thread_node(blk);
			pthread_exit(NULL);
		}
	} else {    //if any word is null exit
		sprintf(str, "|%d|-1|-1|-1|FIN|", FIN_PACKET);  /*send fin packet and close*/

		send_msg_dontwait(blk, str);											  /*connects*/
		destroy_thread_node(blk);
		pthread_exit(NULL);
	}
}

static void NOT_YET_AUTHENTICATED_EXIT(struct thread_block *block, struct packet *pk) {
	if ( block->user_auth == USER_NOT_AUTH ) {
		destroy_thread_node(block); 
		destroy_packet(pk); 
		pthread_cancel(pthread_self()); 
	} 
}

static void interpret_packets(struct thread_block *blk, struct packet *pk) {
	char db_query[MYSQL_QUERY_BUFF+1], str[MAX_PACKET_SIZE+1];
	char *saveptr, *latti_word=NULL, *longi_word=NULL;  //tmp storage for strtok_r calls
	char *mem=NULL; //This is used when dynamic memory is required and it must be freed in the scope used
	
	struct thread_block *node = NULL; //for addrTable
	
	MYSQL_RES *mysql_res = NULL;
	MYSQL_ROW mysql_row;
	
	switch(pk->ptype) {
		case REG_PACKET: register_new_user(blk, pk); break;
		case AUTH_PACKET: authenticate_user(blk, pk); break;
		case CLOSE_PACKET:
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			if ( blk->userid == pk->sender_id ) {
				blk->user_auth = USER_TO_EXIT;
				pthread_cancel( pthread_self() );
			}
			
			destroy_packet(pk);
		break;
		case GET_ALL_USERS_PACKET:
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			snprintf( db_query, MYSQL_QUERY_BUFF,
						"SELECT userid, username FROM %s.users WHERE NOT userid = %d ORDER BY userid ", 
						my_info.database_name, blk->userid );
			
			if ( mysql_query(blk->con, db_query) !=0 ) {
           log_errors( &(blk->tid),
                        MYSQL_ERRORS, 
                        DONT_EXIT, 
                        NO_WRITE, 
                        LOGS_FATAL_ERRORS, 
                        MYSQL_QUERY_FAILED, 
                        NULL, 
                        error_ignore );
			}
			
			mysql_res = mysql_store_result(blk->con);
			if (mysql_res == NULL) {
				mysql_free_result(mysql_res);
		
				log_errors( &(blk->tid),
                        MYSQL_ERRORS, 
                        DONT_EXIT, 
                        NO_WRITE, 
                        LOGS_FATAL_ERRORS, 
                        MYSQL_RESULT_CANT_READ, 
                        NULL, 
                        error_ignore );	
			}
			
			
			while( (mysql_row = mysql_fetch_row(mysql_res)) ) {
				memset(str, '\0', MAX_PACKET_SIZE);
				
				snprintf(str, MAX_PACKET_SIZE,
							"|%d|%d|%d|%d|%s:%s|",
							USERS_PACKET,
							blk->userid, blk->userid,
							COORD_DATA, mysql_row[0], mysql_row[1]);
				
				server_debug("Sending user: %s", str);
				send_msg_dontwait(blk, str);
			}
			
			
		break;
		case MSG_PACKET:
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			if ( pk->receiver_id == blk->userid ) {  //if I am the receiver
				snprintf( str, MAX_PACKET_SIZE,
							"|%d|%d|%d|%d|%s|",          /*format of packet /30/3/1/0/DATA*/
							MSG_PACKET,
							pk->sender_id,
							pk->receiver_id,
							pk->tmsg,
							(char*)pk->msg
						);
						
				send_msg_dontwait(blk, str);
			
			} else {   //i am not the receiver, i am the sender
				pthread_mutex_lock( &addrTable_mutex );
				
				node = getAddrFromaddrTable(pk->receiver_id);
				pthread_mutex_unlock( &addrTable_mutex );
				if ( node != NULL ) {   //if receiver is online
					//append packet to reciver msg queue
					pthread_mutex_lock( &(node->in_queue->queue_lock) );   //get queue_lock to append msg
					enqueue(node->in_queue, pk);     //append msg
					pthread_mutex_unlock( &(node->in_queue->queue_lock) );  
				} else {
					//write to database for messages
					snprintf( db_query, MYSQL_QUERY_BUFF,
								"INSERT INTO %s.messages(sender_id, receiver_id, time_sent, time_received, msg_type, message) \
								VALUES(%d, '%d', '%ld', '%ld', '%d', '%s' )",
								my_info.database_name,    /*The database name*/ 
								pk->sender_id,            /*The sender id*/
								pk->receiver_id,          /*The receiver id*/
								time(NULL),               /*[sent time]Number of seconds since unix epoch*/
								(time_t)-1,               /*[receiver time]Number of seconds since unix epoch*/
								pk->tmsg,                 /*The msg type*/ 
								(char*)pk->msg            /*The msg*/
							);
				
					if ( mysql_query(blk->con, db_query) !=0 ) {
						log_errors( &(blk->tid),
		                       MYSQL_ERRORS, 
		                       DONT_EXIT, 
		                       NO_WRITE, 
		                       LOGS_FATAL_ERRORS, 
		                       MYSQL_QUERY_FAILED, 
		                       NULL, 
		                       error_ignore );
					}
				}
			}
			
			
		break;
		case GET_MSG_PACKET: 
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			if ( blk->userid == pk->sender_id ) {
				sprintf(db_query,
							"SELECT sender_id, receiver_id, msg_type, message FROM %s.messages WHERE \
							receiver_id = %d AND time_received = '-1' ORDER BY time_sent ASC",
							my_info.database_name, blk->userid );
							
				if ( mysql_query(blk->con, db_query) != 0 ) {
						log_errors( &(blk->tid),
		                       MYSQL_ERRORS, 
		                       DONT_EXIT, 
		                       NO_WRITE, 
		                       LOGS_FATAL_ERRORS, 
		                       MYSQL_QUERY_FAILED, 
		                       NULL, 
		                       error_ignore );
				}
				
				mysql_res = mysql_store_result(blk->con);
				if (mysql_res == NULL) {
					mysql_free_result(mysql_res);
		
					log_errors( &(blk->tid),
                        MYSQL_ERRORS, 
                        DONT_EXIT, 
                        NO_WRITE, 
                        LOGS_FATAL_ERRORS, 
                        MYSQL_RESULT_CANT_READ, 
                        NULL, 
                        error_ignore );	
				}
				
				while ( (mysql_row = mysql_fetch_row(mysql_res)) ) {
					memset(str, '\0', MAX_PACKET_SIZE);
					
					snprintf( str, MAX_PACKET_SIZE,
								 "|%d|%s|%s|%s|%s|",
								 MSG_PACKET, 
								 mysql_row[0], mysql_row[1], mysql_row[2], mysql_row[3] );
					
					send_msg_dontwait(blk, str);
				}
				
				mysql_free_result(mysql_res);
				memset(db_query, '\0', MYSQL_QUERY_BUFF);
				
				sprintf( db_query,
							"UPDATE %s.messages SET time_received = '%ld' WHERE receiver_id = %d AND time_received = '-1'",
							my_info.database_name, blk->curr_time() , blk->userid);
							
				if ( mysql_query(blk->con, db_query) != 0 ) {
						log_errors( &(blk->tid),
		                       MYSQL_ERRORS, 
		                       DONT_EXIT, 
		                       NO_WRITE, 
		                       LOGS_FATAL_ERRORS, 
		                       MYSQL_QUERY_FAILED, 
		                       NULL, 
		                       error_ignore );
				}
				
			}
			
			destroy_packet(pk);
		break;
		case GET_GEO_PACKET: 
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			if ( pk->receiver_id == pk->sender_id ) {    /*if I am both the sender and the receiver, then*/ 
				snprintf( str, MAX_PACKET_SIZE,           /*I am asking for my location*/
							"|%d|%d|%d|%d|%ld:%ld|",         /*format of /30/3/1/10/125541:624561/*/
							GEO_PACKET, 
							pk->sender_id, pk->receiver_id, 
							COORD_DATA, blk->loc.longitude, blk->loc.lattitude);
							
				send_msg_dontwait(blk, str);
			}
			
			
			if (blk->userid == pk->receiver_id && pk->sender_id != pk->receiver_id) {  /*if i am the receiver then some is asking */
																                                     /*for my location */
				pthread_mutex_lock( &addrTable_mutex );
				
				node = getAddrFromaddrTable( pk->sender_id );     
				pthread_mutex_unlock( &addrTable_mutex ); 
				
				if ( node != NULL ) {
					memset(str, '\0', MAX_PACKET_SIZE);
					
					snprintf( str, MAX_PACKET_SIZE,
								"|%d|%d|%d|%d|%ld:%ld",
								GEO_PACKET,
								pk->receiver_id,
								pk->sender_id,
								COORD_DATA,
								blk->loc.longitude, blk->loc.lattitude
							 );
					
					struct packet *pckt = string_to_packet(str);  
					
					if ( pckt != NULL ) {
						
						server_debug("Enqueuing packet: |%d|%d|%d|%d|%s|", pckt->ptype, pckt->sender_id, pckt->receiver_id, pckt->tmsg, (char*)pckt->msg);
						
						pthread_mutex_lock( &(node->in_queue->queue_lock) );
						enqueue(node->in_queue, pckt); 
						pthread_mutex_unlock( &(node->in_queue->queue_lock) );
					} else {
						mem = packet_to_string(pk);
						
						sprintf(str, "This packet was dropped: %s", mem);
						if (mem != NULL) free(mem);
						
						log_errors(&(blk->tid),
										STD_ERRORS,
										DONT_EXIT,
										NO_WRITE,
										LOGS_WARNING,
										str,
										NULL,
										error_ignore
									);
					} 
					
				} else {
					/*Receiver not online*/
					mem = packet_to_string(pk);
						
					sprintf(str, "This packet was dropped: %s", mem);
					if (mem != NULL) free(mem);
						
					log_errors(&(blk->tid),
									STD_ERRORS,
									DONT_EXIT,
									NO_WRITE,
									LOGS_WARNING,
									str,
									pk,
									(void(*)(void*))destroy_packet );
				}
					
			}
				
			if (blk->userid == pk->sender_id && pk->sender_id != pk->receiver_id) {    /*if i am the sender, then I am asking for someone location*/
				pthread_mutex_lock( &addrTable_mutex );
				
				node = getAddrFromaddrTable(pk->receiver_id);
				pthread_mutex_unlock( &addrTable_mutex );
				
				if ( node != NULL ) {     //if receiver is online receive geolocation info
					pthread_mutex_lock( &(node->in_queue->queue_lock) );   //lock receiver msg
					
					enqueue(node->in_queue, pk);     //append packet
					pthread_mutex_unlock( &(node->in_queue->queue_lock) );  
					
					server_debug("Enqueue packet: |%d|%d|%d|%d|%s|", pk->ptype, pk->sender_id, pk->receiver_id, pk->tmsg, (char*)pk->msg);
				} else 
					destroy_packet(pk);
			}
			
		break;
		case SET_GEO_PACKET:
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			latti_word = strtok_r(pk->msg, ":", &saveptr);
			longi_word = strtok_r(NULL, ":", &saveptr);
			
			if ( latti_word != NULL && longi_word != NULL ) {
				set_geolocation_info(blk, atol((const char*)longi_word), atol((const char*)latti_word));
			}
			
			destroy_packet(pk);
		break;
		case GEO_PACKET:     //read from the queue
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
				snprintf( str, MAX_PACKET_SIZE,                           
							"|%d|%d|%d|%d|%s|",      /*format of /30/3/1/10/125541:624561/*/
							GEO_PACKET, 
							pk->sender_id, pk->receiver_id, 
							COORD_DATA, (char*)pk->msg);
							
				send_msg_dontwait(blk, str);
		break;
		
	}
}


static void destroy_handlers_block(void *arg) {
	struct thread_block *node = (struct thread_block*)arg;
	
	if ( node != NULL ) {
		pthread_mutex_lock( &addrTable_mutex );
	
		(void*)removeFromAddrTable(node->userid);
		pthread_mutex_unlock( &addrTable_mutex );
	
		destroy_thread_node(node);
	}
}


void *connection_handler(void *data) {
	struct thread_block *thread_node = (struct thread_block*)data;
	struct packet *pk = NULL;
	ssize_t nread = 0;
	
	char buff[MAX_PACKET_SIZE+5];
	int test_cond = -1;   /*check for whether pk is set for [while queue not empty]*/
	
	memset(buff, '\0', MAX_PACKET_SIZE);   //init buff
	
	/*For unmanageble thread cancelling. So the thread_info_block
	* must be search and destroy it manually*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	
	pthread_cleanup_push(destroy_handlers_block, thread_node);   //clean up function
	
	while (1) {
		if ( thread_node->user_auth == USER_TO_EXIT ) break;
		
		//while socket is ready read it
		nread = recv(thread_node->socket, buff, MAX_PACKET_SIZE,  MSG_DONTWAIT); 
		if (nread > 0) {
			pk = string_to_packet(buff); //packetize string packet
			
			server_debug("Read from socket:|%d|%d|%d|%d|%s|", pk->ptype, pk->sender_id, pk->receiver_id, pk->tmsg, (char*)pk->msg);
			if ( pk == NULL ) {   
					strcat(buff, "::packet dropped.");
					
					log_errors( &(thread_node->tid),
				            STD_ERRORS, 
				            DONT_EXIT, 
				            NO_WRITE, 
				            LOGS_WARNING, 
				            buff, 
				            NULL,
				            error_ignore );
			}
			
			interpret_packets(thread_node, pk);
		}
		
		//check whether queue is empty but dont wait if you will block
		pthread_mutex_lock( &(thread_node->in_queue->queue_lock) );
			if ( !empty( thread_node->in_queue ) ) {
				pk = dequeue(thread_node->in_queue);
				test_cond = 0;
				server_debug("Dequeued packet:|%d|%d|%d|%d|%s|", pk->ptype, pk->sender_id, pk->receiver_id, pk->tmsg, (char*)pk->msg);
			}
			
		pthread_mutex_unlock( &(thread_node->in_queue->queue_lock) );
			
			if (test_cond == 0 && pk != NULL) {
				interpret_packets(thread_node, pk);
				server_debug("About to destroy packet:|%d|%d|%d|%d|%s|", pk->ptype, pk->sender_id, pk->receiver_id, pk->tmsg, (char*)pk->msg);
				destroy_packet(pk);    //destroy the packet now
			}
			
			test_cond = -1;
		
		pthread_testcancel();   /*cancellation point*/
		
		memset(buff, '\0', MAX_PACKET_SIZE+5);   //init buff
		sleep(THREAD_WAIT_TIME);   /*cancellation point*/
	}
	
	pthread_cleanup_pop(1);
	mysql_thread_end();
	pthread_exit(NULL);
}



