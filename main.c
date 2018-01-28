#include "server.h"

int main(int argc, char *argv[] ) 
{
	mysql_library_init(0, NULL, NULL);
	
	set_net_data(4040, "127.0.0.1");
	set_mysql_data("localhost", "root", "1993naf", "concurrent_chat");
	make_server();
	
	mysql_library_end();
	return 0;
}
