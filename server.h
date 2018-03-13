/*===========================================================================================
# Copyright (C) 2018 Nafiu Shaibu.
# Purpose: 
#-------------------------------------------------------------------------------------------
# This is a free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your option)
# any later version.

# This is distributed in the hopes that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

#===========================================================================================
*/

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

#include "./libs/generic_linked_list.h"
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
#define MYSQL_QUERY_BUFF 500
#endif

#if !defined ( NAME_BUF_SIZE ) || ! defined ( PASSWD_BUF_SIZE ) || ! defined ( MSG_BUFF_SIZE )
#define NAME_BUF_SIZE 100
#define PASSWD_BUF_SIZE 120
#define MSG_BUFF_SIZE 1000
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


void set_net_data(int port, const char *ip);

void set_mysql_data(const char *serv, const char *user, const char *pwd, const char *dbname);

void make_server(void);

void close_socket(void);

#endif
