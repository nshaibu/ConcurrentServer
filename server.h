#ifndef SERVER_H
#define SERVER_H

#define _REENTRANT

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./data_structs/thread_info_block.h"
#include "./libs/iterator.h"

#undef _TRY_

#ifdef SOMAXCONN
#define MAX_CONN SOMAXCONN
#else
#define MAX_CONN 100
#endif

typedef struct net_info {
    char ip_addr[16];
    int port;
    int socket;
} *serverDataPtr, serverData;


struct mysql_info {
	char server_name[20];
	char user_name[20];
	char user_password[20];
	char database_name[20];
};

void set_net_data(int port, const char *ip);

void set_mysql_data(const char *serv, const char *user, const char *pwd, const char *dbname);

void make_server(void);

void close_socket(void);

#endif
