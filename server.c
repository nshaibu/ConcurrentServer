#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql.h>

#include "server.h"

static struct sockaddr_in serv_addr;	//server address struct

static struct sockaddr_in cli_addr;		//client address struct

static serverData net_info;			//server informations

static struct mysql_info my_info;		//mysql server connection information

static int kill_server = 0;					/*if none zero, kill server*/



static void *connection_handler(void *data) {
	struct thread_block *thread_node = (struct thread_block*)data;
	int i = 0, client = thread_node->curr_time();
	char buff[20];
	
	sprintf(buff, "%s::%d", "hello world", client);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	while (1) {
		//sprintf(buff, "client on server are %d", get_list_height());
		//send(thread_node->data->threads_socket, buff, sizeof(buff), MSG_DONTWAIT);
		send(thread_node->socket, buff, strlen(buff), MSG_DONTWAIT);
		sleep(4);
		
		if (i == 10) break;
		++i;
	}
	shutdown(thread_node->socket, 2);
	//close(thread_node->data->threads_socket);
	pthread_cancel(pthread_self());
}

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


static void sig_handler(int sig) {
	++kill_server;
}

void make_server() {
	struct thread_block *thr_node; //Thread info nodes
	iterPtr iter_node;
	
	MYSQL *mysql_con;			//main mysql server connection handler
	MYSQL_RES *mysql_res;
	char db_query[100];
	
	int connection_socket, ret, size;
	int thread_count = 0;   //thread count
	
	pthread_t tid[MAX_CONN];
	pthread_attr_t pattr;		//pthread attributes
	
	struct sigaction sa;
	
	mysql_con = mysql_init(NULL);
	if (mysql_con == NULL) {
		fprintf(stderr, "%s\n", mysql_error(mysql_con));
		exit(EXIT_FAILURE);
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
			fprintf(stderr, "%s\n", mysql_error(mysql_con));
			exit(EXIT_FAILURE);
	}
	
	//check whether database exist
	sprintf( db_query, 
            "SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME = '%s'",
            my_info.database_name
          );
			
	if ( mysql_query(mysql_con, db_query) != 0 ) {
		fprintf(stderr, "%s\n", mysql_error(mysql_con));
		exit(EXIT_FAILURE);
	}
	
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
                username VARCHAR(20) NOT NULL, \
                userpasswd VARCHAR(20) NOT NULL \
               )",
               my_info.database_name
             );
		mysql_query(mysql_con, db_query);
		
		memset(db_query, '\0', sizeof(db_query));
		sprintf( db_query,
               "CREATE TABLE %s.messages ( \
                msg_userid INTEGER NOT NULL, \
                message VARCHAR(100) NOT NULL \
               )",
               my_info.database_name
				 );
		mysql_query(mysql_con, db_query);
		
		mysql_free_result(mysql_res);
	}
	
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = sig_handler;
	
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		perror("sigaction");
		exit(EXIT_FAILURE);
	}
	
	
	if ( pthread_attr_init(&pattr) ) {
		pthread_attr_setdetachstate(&pattr, PTHREAD_CREATE_JOINABLE);
		pthread_attr_setschedpolicy(&pattr, SCHED_OTHER);
		pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);
		pthread_attr_setinheritsched(&pattr, PTHREAD_EXPLICIT_SCHED);
		
	}
	
	if ( (net_info.socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) { 
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(net_info.port);
	serv_addr.sin_addr.s_addr = inet_addr(net_info.ip_addr);
	
	
	if ( bind(net_info.socket, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0 ) {
		perror("bind");
		exit(EXIT_FAILURE);
	}
	
	listen(net_info.socket, MAX_CONN);
	
	while(1) {
		if ( kill_server != 0 ) break;
		
		size = sizeof(cli_addr);
		connection_socket = accept(net_info.socket, (struct sockaddr*)&cli_addr, (socklen_t*)&size);
		
		thr_node = create_thread_node(-1, connection_socket);
		
		if ( thr_node != NULL ) {
			
			if ( thread_count <= MAX_CONN ) {
				ret = pthread_create(&tid[thread_count], &pattr, connection_handler, (void*)thr_node); 
				if ( ret != 0 ) {
					fprintf(stderr, "%s %s %d\n", "Failed to start handler", __FILE__, __LINE__);
					destroy_thread_node(thr_node);
				
				}else {
					set_thread_node_tid(thr_node, tid[thread_count]); 
				}
				
			} else {
				destroy_thread_node(thr_node);
			}
			
			thread_count++;
		}
	}
	
	for (int i=0; i<thread_count; i++)
		pthread_cancel(tid[i]);
	
	for (int i=0; i<thread_count; i++) 
		pthread_join(tid[i], NULL);
	
	pthread_attr_destroy(&pattr);
	
	mysql_close(mysql_con);
	close(net_info.socket);
}


#ifdef _TRY_
	
	int main(int argc, char **argv) {
	
		set_net_data(4040, "127.0.0.1");
		set_mysql_data("localhost", "root", "1993naf", "concurrent_chat");
		make_server();
		
		//sem_destroy(get_bin_semphore());
		return 0;
	}
#endif
