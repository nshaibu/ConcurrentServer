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

#include "server.h"


int main(int argc, char *argv[] ) 
{
	mysql_library_init(0, NULL, NULL);
	

	if ( argc < 6 )
		return 0;

	init_logs_object();
	
	unsigned int port = (unsigned)atoi(argv[1]);

	set_net_data(port, argv[2]);
	set_mysql_data(argv[3], argv[4], argv[5], "concurrent_chat");
	make_server();
	
	mysql_library_end();
	return 0;
}
