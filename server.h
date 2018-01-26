#ifndef SERVER_H
#define SERVER_H

#define _REENTRANT

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "./data_structs/thread_info_block.h"
#include "./libs/iterator.h"

#define _TRY_

typedef struct net_info {
    char ip_addr[16];
    int port;
    int socket;
} *serverDataPtr, serverData;


void set_net_data(int port, const char *ip);
//sem_t *get_bin_semphore();

void make_server(void);

void close_socket(void);

#endif
