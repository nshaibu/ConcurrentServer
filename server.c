#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql.h>

#include "server.h"

static struct sockaddr_in serv_addr;	//server address struct

static struct sockaddr_in cli_addr;		//client address struct

static serverData net_info;			//server informations

struct mysql_info my_info;		//mysql server connection information

static int kill_server = 0;					/*if none zero, kill server*/


void set_net_data(int port, const char *ip)
{
	net_info.socket = -1;
	net_info.port = port;
	strcpy(net_info.ip_addr, ip);
}


void set_mysql_data(const char *serv, const char *user, const char *pwd, const char *dbname) 
{
	strcpy(my_info.server_name, serv);
	strcpy(my_info.user_name, user);
	strcpy(my_info.user_password, pwd);
	strcpy(my_info.database_name, dbname);
}


static void sig_handler(int sig) {
	kill_server = 23;
}

void make_server() 
{
	struct thread_block *thr_node = NULL; //Thread info nodes
	Generic_list *thread_list = NULL;   //list of thread ids
	
	MYSQL *mysql_con = NULL;			//main mysql server connection handler
	MYSQL_RES *mysql_res = NULL;
	char db_query[MYSQL_QUERY_BUFF];
	
	int connection_socket, ret, size;
	//int thread_count = 0;   //thread count
	
	struct list_node *thread_id_node = NULL;
	pthread_attr_t pattr;		//pthread attributes
	
	struct sigaction sa;
	
	thread_list = create_generic_list(ENABLE_LOCK);
	if (thread_list == NULL) {
		log_errors( NULL,
					STD_ERRORS,
					DO_EXIT,
					WRITE_STDDER,
					LOGS_FATAL_ERRORS,
					"Cannot initialize generic list for thread IDs",
					NULL,
					error_ignore
				  );
	} else
		SERVER_MSG("Initialized generic list for thread ID");

	mysql_con = mysql_init(NULL);
	if (mysql_con == NULL) {
		log_errors( NULL,
                  	MYSQL_ERRORS, 
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	MYSQL_INIT_FAILED, 
                  	mysql_con,
                  	(void (*)(void*))mysql_close);
   }

	SERVER_MSG("Connecting to database server on host [%s]...", my_info.server_name);
	if (! mysql_real_connect( mysql_con,
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
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	MYSQL_CONNECTION_FAILED, 
                  	mysql_con,
                  	(void (*)(void*))mysql_close);
	} else
		SERVER_MSG("Connected to database server on host [%s]", my_info.server_name);
	
	//check whether database exist
	sprintf( db_query, 
            "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '%s'",
            my_info.database_name
          );
			
	if ( mysql_query(mysql_con, db_query) != 0 ) {
		log_errors( NULL,
                  	MYSQL_ERRORS, 
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	MYSQL_QUERY_FAILED, 
                  	mysql_con, 
                  	(void (*)(void*))mysql_close);
	} else 
		SERVER_MSG("Checked whether database [%s] exist on host [%s].", my_info.database_name, my_info.server_name);
	
	//check result of the query executed
	mysql_res = mysql_store_result(mysql_con);
	int test = mysql_num_rows(mysql_res);
	
	if (test == 0) {
		//then create the database and tables again
		
		memset(db_query, '\0', sizeof(db_query));
		sprintf( db_query, "CREATE DATABASE %s", my_info.database_name);
		mysql_query(mysql_con, db_query);
		
		memset(db_query, '\0', sizeof(db_query));
		sprintf( db_query,
               "CREATE TABLE %s.users ( \
                userid INTEGER NOT NULL AUTO_INCREMENT PRIMARY KEY, \
                username VARCHAR(%d) NOT NULL, \
                userpasswd VARCHAR(%d) NOT NULL \
               )",
               my_info.database_name, NAME_BUF_SIZE, PASSWD_BUF_SIZE );
			   
		if ( mysql_query(mysql_con, db_query) !=0) {
			log_errors( NULL,
                  		MYSQL_ERRORS, 
                  		DO_EXIT, 
                  		WRITE_STDDER, 
                  		LOGS_FATAL_ERRORS, 
                  		MYSQL_QUERY_FAILED, 
                  		mysql_con, 
                  		(void (*)(void*))mysql_close);
		} else 
			SERVER_MSG("Created database [%s] on host [%s]", my_info.database_name, my_info.server_name);
		
		memset(db_query, '\0', sizeof(db_query));
		sprintf( db_query,
               "CREATE TABLE %s.messages ( \
                sender_id INTEGER NOT NULL, \
                receiver_id INTEGER NOT NULL, \
                time_sent DECIMAL(20, 0), \
                time_received DECIMAL(20, 0), \
                msg_type INTEGER NOT NULL, \
                message VARCHAR(%d) NOT NULL \
               )",
               my_info.database_name, MAX_DATA_SIZE );	/*time_sent and time_received is measured the unix epoch*/
				 
		if ( mysql_query(mysql_con, db_query) !=0 ) {
			log_errors( NULL,
                  		MYSQL_ERRORS, 
                  		DO_EXIT, 
                  		WRITE_STDDER, 
                  		LOGS_FATAL_ERRORS, 
                  		MYSQL_QUERY_FAILED, 
                  		mysql_con, 
                  		(void (*)(void*))mysql_close);
		}
		
		server_debug("Created database %s.", my_info.database_name);
		
		mysql_free_result(mysql_res);
	}
	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	
	if ( sigaction(SIGINT, &sa, NULL) == -1 ) {
		log_errors( NULL,
                  	STD_ERRORS, 
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	SIGNAL_FAILED, 
                  	NULL, 
                  	error_ignore);
	}
	
	
	if ( pthread_attr_init(&pattr) == 0 ) {
		pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setschedpolicy(&pattr, SCHED_OTHER);
		pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
		pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);
		
	}
	
	if ( (net_info.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		log_errors( NULL,
                  	STD_ERRORS, 
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	SOCKET_NOT_CREATED, 
                  	&(net_info.socket), 
                  	error_close_fd);
	} else
		SERVER_MSG("Creating a TCP socket...");
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(net_info.port);
	serv_addr.sin_addr.s_addr = inet_addr(net_info.ip_addr);
	
	
	if ( bind(net_info.socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) {
		log_errors( NULL,
                  	STD_ERRORS, 
                  	DO_EXIT, 
                  	WRITE_STDDER, 
                  	LOGS_FATAL_ERRORS, 
                  	SOCKET_BINDING_FAILED, 
                  	&(net_info.socket), 
                  	error_close_fd);
	} else 
		SERVER_MSG("Binding socket to IP:[%s] and port number:[%d]...", net_info.ip_addr, net_info.port);
	
	SERVER_MSG("Listening on port:[%d] for connection...", net_info.port);
	listen(net_info.socket, MAX_CONN);
	
	while(1) {
		if ( kill_server != 0 ) break;
		
		size = sizeof(cli_addr);
		connection_socket = accept(net_info.socket, (struct sockaddr*)&cli_addr, (socklen_t*)&size);
		if ( connection_socket < 0 ) {
			
			if (errno != EINTR) {
				log_errors( NULL,
                	     	STD_ERRORS, 
                    	 	DONT_EXIT, 
                    	 	WRITE_STDDER, 
                    	 	LOGS_WARNING, 
                     	 	SOCKET_NOT_CREATED, 
                     	 	NULL, 
                     	 	error_ignore );
				
				continue;
			} else 
				break;

		} else 
			SERVER_MSG("User connecting to the server. %d users have connected.", thread_list->list_len);
		
		
		thread_id_node = create_list_node(sizeof(pthread_t));
		if (thread_id_node == NULL) {
			shutdown(connection_socket, SHUT_RDWR);

			errno = ENOMEM;
			log_errors( NULL,
						STD_ERRORS,
						DONT_EXIT,
						WRITE_STDDER,
						LOGS_INFO,
						"Cannot create list nodes",
						&connection_socket,
						error_close_fd );

			continue;
		}

		thr_node = create_thread_node(NULL, connection_socket, thread_list);
		
		if ( thr_node != NULL ) {

			ret = pthread_create((pthread_t*)thread_id_node->data, &pattr, connection_handler, (void*)thr_node); 
			if ( ret != 0 ) {
				log_errors( NULL,
                           	STD_ERRORS, 
                           	DONT_EXIT, 
                           	WRITE_STDDER, 
                           	LOGS_WARNING, 
                           	THREAD_NOT_START, 
                           	NULL, 
                           	error_ignore );

				destroy_thread_node(thr_node);
			} else 
				set_thread_node_tid(thr_node, thread_id_node);
				
		} else {
			shutdown(connection_socket, SHUT_RDWR);
			
			errno = ENOMEM;
			log_errors( NULL,
                  		STD_ERRORS, 
                  		DONT_EXIT, 
                  		WRITE_STDDER, 
                  		LOGS_INFO, 
                  		"Cannot create thread information block(TIB)", 
                  		&connection_socket, 
                  		error_close_fd );

			free(thread_id_node->data);
			free(thread_id_node);
		}

	}
	
	SERVER_MSG("Server shutting down...");

	while (thread_list->list_len > 0) 
	{
		pthread_t *data = (pthread_t*)list_popfront(thread_list);
		if (data != NULL) { 
			pthread_cancel(*data);
			pthread_join(*data, NULL);
		}
	}

	list_lock_acquire( thread_list );
	destroy_generic_list( thread_list, free);
	
	pthread_attr_destroy(&pattr);
	
	mysql_close(mysql_con);
	destroy_logs_object();
	close(net_info.socket);
	
	SERVER_MSG("Server Shutdown!");
}


#ifdef _TRY_
	
	int main(int argc, char **argv) {
	
		set_net_data(4040, "127.0.0.1");
		set_mysql_data("localhost", "root", "database_password", "concurrent_chat");
		make_server();
		
		return 0;
	}
#endif
