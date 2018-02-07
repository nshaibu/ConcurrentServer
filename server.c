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


void set_net_data(int port, const char *ip) {
	net_info.socket = -1;
	net_info.port = port;
	strcpy(net_info.ip_addr, ip);
}


void set_mysql_data(const char *serv, const char *user, const char *pwd, const char *dbname) {
	strcpy(my_info.server_name, serv);
	strcpy(my_info.user_name, user);
	strcpy(my_info.user_password, pwd);
	strcpy(my_info.database_name, dbname);
}

/*get mysql server connection info from any file*/
/*const struct mysql_info *get_mysql_info_struct() {*/
/*	return &my_info;*/
/*}*/


static void sig_handler(int sig) {
	kill_server = 23;
}

void make_server() {
	struct thread_block *thr_node = NULL; //Thread info nodes
	
	MYSQL *mysql_con = NULL;			//main mysql server connection handler
	MYSQL_RES *mysql_res = NULL;
	char db_query[MYSQL_QUERY_BUFF];
	
	int connection_socket, ret, size;
	int thread_count = 0;   //thread count
	
	pthread_t tid[MAX_CONN];
	pthread_attr_t pattr;		//pthread attributes
	
	struct sigaction sa;
	
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
	}
	
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
	}
	
	server_debug("Checked whether %s exist.", my_info.database_name);
	
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
               my_info.database_name, NAME_BUF_SIZE, PASSWD_BUF_SIZE
             );
		if ( mysql_query(mysql_con, db_query) !=0) {
			log_errors( NULL,
                  MYSQL_ERRORS, 
                  DO_EXIT, 
                  WRITE_STDDER, 
                  LOGS_FATAL_ERRORS, 
                  MYSQL_QUERY_FAILED, 
                  mysql_con, 
                  (void (*)(void*))mysql_close);
		}
		
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
               my_info.database_name, MAX_DATA_SIZE
				 );	/*time_sent and time_received is measured the unix epoch*/
				 
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
	}
	
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
	}
	
	listen(net_info.socket, MAX_CONN);
	
	while(1) {
		if ( kill_server != 0 ) break;
		
		size = sizeof(cli_addr);
		connection_socket = accept(net_info.socket, (struct sockaddr*)&cli_addr, (socklen_t*)&size);
		if ( connection_socket < 0 ) {
			log_errors( NULL,
                     STD_ERRORS, 
                     DONT_EXIT, 
                     NO_WRITE, 
                     LOGS_WARNING, 
                     SOCKET_NOT_CREATED, 
                     &connection_socket, 
                     error_close_fd );
		}
		
		thr_node = create_thread_node(-1, connection_socket);
		
		if ( thr_node != NULL ) {
			
			if ( thread_count <= MAX_CONN ) {
				ret = pthread_create(&tid[thread_count], &pattr, connection_handler, (void*)thr_node); 
				if ( ret != 0 ) {
					log_errors( NULL,
                           STD_ERRORS, 
                           DONT_EXIT, 
                           NO_WRITE, 
                           LOGS_WARNING, 
                           THREAD_NOT_START, 
                           NULL, 
                           error_ignore );
					destroy_thread_node(thr_node);
				
				} else {
					set_thread_node_tid(thr_node, tid[thread_count]); 
				}
				
			} else {
					log_errors( NULL,
                  STD_ERRORS, 
                  DONT_EXIT, 
                  NO_WRITE, 
                  LOGS_INFO, 
                  MAX_CON_REACH, 
                  thr_node, 
                  (void(*)(void*))destroy_thread_node);
			}
			
			thread_count++;
		}
	}
	
	//destroy_addrTable();   //asked the threads to stop
	
	for (int i=0; i<thread_count; i++) pthread_cancel(tid[i]);
	for (int i=0; i<thread_count; i++) pthread_join(tid[i], NULL);
	pthread_attr_destroy(&pattr);
	
	mysql_close(mysql_con);
	destroy_logs_object();
	close(net_info.socket);
}


#ifdef _TRY_
	
	int main(int argc, char **argv) {
	
		set_net_data(4040, "127.0.0.1");
		set_mysql_data("localhost", "root", "1993naf", "concurrent_chat");
		make_server();
		
		return 0;
	}
#endif
