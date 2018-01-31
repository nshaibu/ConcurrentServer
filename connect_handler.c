#include "connect_handler.h"

static struct mysql_info *my_info = NULL; 

static int authenticator(struct thread_block *blk, const char *name, const char *passwd) {
	char db_query[MYSQL_QUERY_BUFF];
	MYSQL_RES *mysql_res = NULL;
	MYSQL_ROW mysql_row;
	
	my_info = (struct mysql_info*)get_mysql_info_struct();   //set mysql info here
	
	blk->con = mysql_init(NULL);
	if ( blk->con == NULL ) {
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_INIT_FAILED, 
                  blk,
                  (void (*)(void*))destroy_thread_node);
	}
	
	if (! mysql_real_connect( blk->con,
                             my_info->server_name,
                             my_info->user_name,
                             my_info->user_password,
                             NULL,
                             0, 
                             NULL,
                             0
									)
	) {
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_CONNECTION_FAILED, 
                  blk,
                  (void (*)(void*))destroy_thread_node );
	}
	
	sprintf( db_query, 
			   "SELECT userid, username FROM %s.users WHERE username = '%s' AND userpasswd = '%s'",
			   my_info->database_name, name, passwd);
	
	if ( mysql_query(blk->con, db_query) !=0 ) {
		log_errors( &(blk->tid),
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
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
                  WRITE_STDDER, 
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
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_NO_USER, 
                  blk, 
                  (void (*)(void*))destroy_thread_node );	
	}
	
	mysql_free_result(mysql_res);
	blk->userid = atoi(mysql_row[0]);   //set thread block user id 
	strcpy(blk->user_name, mysql_row[1]);   //set thread block user name
	
	insertInAddrTable(blk->userid, (void*)blk);   //put userid and thread_block address in address table
	
	blk->user_auth = USER_AUTH;   //set that user has been authenticated
	return EXIT_SUCCESS;
}


void *connection_handler(void *data) {
	struct thread_block *thread_node = (struct thread_block*)data;
	ssize_t nread = 0;
	char buff[MAX_PACKET_SIZE+1];
	
	memset(buff, '\0', MAX_PACKET_SIZE+1);   //init buff
	
	/*For unmanageble thread cancelling. So the thread_info_block
	* must be search and destroy it manually*/
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
	
	while (1) {
		if ( thread_node->user_auth == USER_TO_EXIT ) break;   //exit thread
		
		nread = recv(thread_node->socket, buff, MAX_PACKET_SIZE,  MSG_DONTWAIT);   /*cancellation point*/
		if ( nread < 0 ) {    //if reading error occurred then do following
			
			switch(errno) {
				case EINTR:
				case ENOMEM:
				case ECONNREFUSED:
					log_errors( &(thread_node->tid),
				            STD_ERRORS, 
				            DONT_EXIT, 
				            NO_WRITE, 
				            LOGS_WARNING, 
				            "Recv:error reading from socket", 
				            NULL,
				            error_ignore);
				break;
				
				default:
					if (thread_node->userid != -1) //code for removal from addrtable
						(void)removeFromAddrTable(thread_node->userid);
					
					log_errors( &(thread_node->tid),
				            STD_ERRORS, 
				            DO_EXIT, 
				            NO_WRITE, 
				            LOGS_FATAL_ERRORS, 
				            "Recv::error reading from socket", 
				            thread_node,
				            (void(*)(void*))destroy_thread_node );
			}
		}
		else if ( nread > 0 ) {
			struct packet *pk = string_to_packet(buff); //packetize string packet
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
			
			if ( thread_node->user_auth == USER_NOT_AUTH ) {
				if ( pk->ptype == NEG_PACKET ) {
					
					if ( pk->msg_type != COORD_DATA ) {
						log_errors( &(thread_node->tid),
				            STD_ERRORS, 
				            DONT_EXIT, 
				            NO_WRITE, 
				            LOGS_WARNING, 
				            "packet not in right format", 
				            pk,
				            (void(*)(void*))destroy_packet );
					} else {
						char passwd[PASSWD_BUF_SIZE];
						char name[NAME_BUF_SIZE];
						
						
						
						(void)authenticator(thread_node, );
					}
				}
			}
			memset(buff, '\0', MAX_PACKET_SIZE+1);   //clear buffer
		}
		
		//while input msg queue is not empty
		if ( !empty( thread_node->in_queue ) ) {
			pthread_mutex_lock( &(thread_node->in_queue->queue_lock) );   //lock queue mutex to edit
			
			struct packet *pk = dequeue( thread_node->in_queue );
			//process packet and respond
		}
		
	}
	
	mysql_thread_end();
}
