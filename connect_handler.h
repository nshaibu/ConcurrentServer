#ifndef CONNECTION_HANDLER
#define CONNECTION_HANDLER

#include "./libs/queue.h"
#include "./data_structs/thread_info_block.h"
#include "./data_structs/addr_table.h"
#include "./data_structs/packets.h"
#include "./error_logs/errno_logs.h"


void *connection_handler(void *data);


#endif

