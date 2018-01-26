#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"

static struct sockaddr_in serv_addr; //server address struct
static struct sockaddr_in cli_addr; //client address struct

static serverData net_info;

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
		
		if (i == 5) break;
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

void make_server() {
	struct thread_block *thr_node; //Thread info nodes
	iterPtr iter_node;
	
	if ( sem_init(&global_sem, 0, 0) != 0 ) {
		perror("sem_init");
		exit(EXIT_FAILURE);
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
	
	listen(net_info.socket, SOMAXCONN);
	int connection_socket, ret;
	
	while(1) {
		int size = sizeof(cli_addr);
		connection_socket = accept(net_info.socket, (struct sockaddr*)&cli_addr, (socklen_t*)&size);
		
		pthread_t tid;
		thr_node = create_thread_node(-1, connection_socket);
		
		if ( thr_node != NULL) {
			ret = pthread_create(&tid, NULL, connection_handler, (void*)thr_node); 
			if (ret != 0) {
				fprintf(stderr, "%s %s %d\n", "Failed to start handler", __FILE__, __LINE__);
				destroy_thread_node(thr_node);
				
			}else {
				set_thread_node_tid(thr_node, tid);
				
				//iter_node = create_iterator(thr_node);
				//link_iterator(iter_node);
			}
			
		}
	}
}


#ifdef _TRY_
	
	int main(int argc, char **argv) {
	
		set_net_data(4020, "127.0.0.1");
		make_server();
		
		sem_destroy(get_bin_semphore());
		return 0;
	}
#endif
