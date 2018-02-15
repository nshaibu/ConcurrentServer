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

