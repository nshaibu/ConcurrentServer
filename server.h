#ifndef SERVER_H
#define SERVER_H

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./error_logs/errno_logs.h"
#include "./data_structs/thread_info_block.h"
#include "./data_structs/addr_table.h"
#include "./data_structs/packets.h"
#include "connect_handler.h"

#ifdef SOMAXCONN
#define MAX_CONN SOMAXCONN
#else
#define MAX_CONN 100
#endif

#ifndef MYSQL_QUERY_BUFF
#define MYSQL_QUERY_BUFF 100
#endif

#if !defined ( NAME_BUF_SIZE ) || ! defined ( PASSWD_BUF_SIZE )
#define NAME_BUF_SIZE 50
#define PASSWD_BUF_SIZE 70
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

extern struct mysql_info my_info;

//const struct mysql_info *get_mysql_info_struct();   //get mysql information from here

void set_net_data(int port, const char *ip);

void set_mysql_data(const char *serv, const char *user, const char *pwd, const char *dbname);

void make_server(void);

void close_socket(void);

#endif
