#ifndef CONNECTION_HANDLER
#define CONNECTION_HANDLER

#include <mysql.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "server.h"
#include "./libs/queue.h"
#include "./data_structs/thread_info_block.h"
#include "./data_structs/addr_table.h"
#include "./data_structs/packets.h"
#include "./error_logs/errno_logs.h"

#define THREAD_WAIT_TIME 0.51223   /*The time for the connect_handler thread to wait*/

#ifndef MYSQL_QUERY_BUFF
#define MYSQL_QUERY_BUFF 500
#endif


//static int authenticator(struct thread_block *blk, const char *name, const char *passwd);

void *connection_handler(void *data);


#endif

