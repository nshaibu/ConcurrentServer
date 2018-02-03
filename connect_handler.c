#include "connect_handler.h"

static void send_msg_dontwait(struct thread_block *blk, const char *msg){
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
	char passwd[PASSWD_BUF_SIZE], name[NAME_BUF_SIZE], str[MAX_DATA_SIZE];
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
			sprintf(str, "|%d|-1|-1|-1|A|", ACK_PACKET);
			send_msg_dontwait(blk, str);
		} else {    //authentication failed
			sprintf(str, "|%d|-1|-1|-1|A|", FIN_PACKET);
			send_msg_dontwait(blk, str);

			destroy_thread_node(blk);
			pthread_exit(NULL);
		}
	} else {    //if any word is null exit
		sprintf(str, "|%d|-1|-1|-1|A|", FIN_PACKET);  /*send fin packet and close*/

		send_msg_dontwait(blk, str);											  /*connects*/
		destroy_thread_node(blk);
		pthread_exit(NULL);
	}
}



static void register_new_user(struct thread_block *blk, struct packet *pk) {
	char passwd[PASSWD_BUF_SIZE], name[NAME_BUF_SIZE], str[MAX_DATA_SIZE];
	char *saveptr, *pword = NULL, *nword = NULL;
	
	//May have email address
	
	strcpy(str, pk->msg);
	destroy_packet(pk);
	
	nword = strtok_r(str, ":", &saveptr);
	pword = strtok_r(NULL, ":", &saveptr);
	
	if ( nword != NULL && pword != NULL ) {
		strcpy(name, nword);
		strcpy(passwd, pword);
		
		int ret = registrator(blk, name, passwd);
		if ( ret == 0 ) {
			sprintf(str, "|%d|-1|-1|-1|A|", ACK_PACKET );
			send_msg_dontwait(blk, str);
		} else {    //authentication failed
			sprintf(str, "|%d|-1|-1|-1|A|", FIN_PACKET);
			send_msg_dontwait(blk, str);

			destroy_thread_node(blk);
			pthread_exit(NULL);
		}
	} else {    //if any word is null exit
		sprintf(str, "|%d|-1|-1|-1|A|", FIN_PACKET);  /*send fin packet and close*/

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
	char db_query[MYSQL_QUERY_BUFF], str[MAX_DATA_SIZE];
	char *saveptr, *latti_word=NULL, *longi_word=NULL;  //tmp storage for strtok_r calls
	struct thread_block *node = NULL; //for addrTable
	
	
	switch(pk->ptype) {
		case REG_PACKET: register_new_user(blk, pk); break;
		case AUTH_PACKET: authenticate_user(blk, pk); break;
		case MSG_PACKET:
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			node = getAddrFromaddrTable(pk->receiver_id);
			if ( node != NULL ) {
				//append packet to reciver msg queue
				if ( pk->receiver_id == blk->userid) {  //if I am the receiver
					sprintf( str,
								"|%d|%d|%d|%d|%s",
								MSG_PACKET,
								pk->sender_id,
								pk->receiver_id,
								pk->tmsg,
								(char*)pk->msg
							);
						
					send_msg_dontwait(node, str);
				}
			} else {
				//write to database for messages
				sprintf( db_query,
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
                          DO_EXIT, 
                          NO_WRITE, 
                          LOGS_FATAL_ERRORS, 
                          MYSQL_QUERY_FAILED, 
                          blk, 
                          (void (*)(void*))destroy_thread_node );
				}
			}
			
			destroy_packet(pk);
		break;
		case GET_GEO_PACKET: 
			NOT_YET_AUTHENTICATED_EXIT(blk, pk);
			
			if ( pk->receiver_id == pk->sender_id ) {
				sprintf( str, 
							"|%d|%d|%d|%d|%ld:%ld|", 
							GEO_PACKET, 
							pk->sender_id, pk->receiver_id, 
							COORD_DATA, blk->loc.longitude, blk->loc.lattitude);
							
				send_msg_dontwait(blk, str);
			} else {  //get others geolocation
				node = getAddrFromaddrTable(pk->receiver_id);
				if ( node != NULL ) {     //if receiver is online receive geolocation info
					pthread_mutex_lock( &(blk->in_queue->queue_lock) );   //lock receiver msg
				
					enqueue(blk->in_queue, pk);     //append packet
					pthread_mutex_unlock( &(blk->in_queue->queue_lock) );  
				}
			}
			destroy_packet(pk);
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
		
	}
}

#ifndef TRY_CON

void *connection_handler(void *data) {
	struct thread_block *thread_node = (struct thread_block*)data;
	struct packet *pk = NULL;
	ssize_t nread = 0;
	char buff[MAX_PACKET_SIZE+1];
	int test_cond = -1;   /*check for whether pk is set for [while queue not empty]*/
	memset(buff, '\0', MAX_PACKET_SIZE+1);   //init buff
	
	/*For unmanageble thread cancelling. So the thread_info_block
	* must be search and destroy it manually*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	while (1) {
		if ( thread_node->user_auth == USER_TO_EXIT ) break;
		
		//while socket is ready read it
		nread = recv(thread_node->socket, buff, MAX_PACKET_SIZE,  MSG_DONTWAIT); 
		if (nread > 0) {
			pk = string_to_packet(buff); //packetize string packet
			
			if ( pk == NULL ) {   
					strcat(buff, "::packet droped.");
					
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
		if ( pthread_mutex_trylock( &(thread_node->in_queue->queue_lock) ) == 0 ) {
			if ( !empty( thread_node->in_queue ) ) {
				pk = dequeue(thread_node->in_queue);
				test_cond = 0;
			}
			pthread_mutex_unlock( &(thread_node->in_queue->queue_lock) );
			
			if (test_cond == 0) 
				interpret_packets(thread_node, pk);
			else 
				test_cond = -1;
		}
		
		sleep(0.43);
	}
	
	mysql_thread_end();
	pthread_exit(0);
}


#else


void *connection_handler(void *data) {
	struct thread_block *thread_node = (struct thread_block*)data;
	int i = 0;
	char buff[50];
	
/*	MYSQL_RES *res;*/
/*	MYSQL_ROW srow;*/
	
	//sprintf(buff, "%s", "hello world");
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	
/*	  if (mysql_query(thread_node->con, "select userid, userpasswd from concurrent_chat.users") ){*/
/*      printf("%s\n", mysql_error(thread_node->con));*/
/*      pthread_exit(0);*/
/*    }*/


/*    res = mysql_use_result(thread_node->con);*/
/*    if (res) {*/
/*      while ( (srow = mysql_fetch_row(res)) ) {*/
/*        sprintf(buff, "%s %s\n", srow[0], srow[1]);*/
/*      }*/

/*      mysql_free_result(res);*/
/*    }*/
	authenticator(thread_node, "nuhu", "556mnj");
	sprintf(buff, "%d", get_addr_table_len());
	while (1) {
		//sprintf(buff, "client on server are %d", get_list_height());
		send(thread_node->socket, buff, strlen(buff), MSG_DONTWAIT);
		//recv(thread_node->socket, buff, 50,  MSG_DONTWAIT);
		send(thread_node->socket, thread_node->username, strlen(thread_node->username), MSG_DONTWAIT);
		sleep(4);
		
		//if (thread_node->user_auth == USER_TO_EXIT) break;
		
		if (i == 5) break;
		++i;
	}
	//write(thread_node->socket, "Got here\n", 10);
	shutdown(thread_node->socket, 2);
	//close(thread_node->socket);
	//pthread_cancel(pthread_self());
	destroy_thread_node(thread_node);
	mysql_thread_end();
	return NULL;
}


#endif
