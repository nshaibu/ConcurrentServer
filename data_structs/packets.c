#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "packets.h"

static char *make_message(const char *fmt, ...) {
	int size = 0;
	char *p = NULL;
	va_list ap;

	/* Determine required size */
	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);

	if (size < 0)
		return NULL;

	size++;             /* For '\0' */
	p = malloc(size);
	if (p == NULL)
		return NULL;

	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	if (size < 0) {
		free(p);
			return NULL;
	}
	va_end(ap);

	return p;
}


char *pk_deencapsulator(const char *str, int field_to_ret) {
	if ( str == NULL )
		return NULL;
	int i=0, num_of_bar=0, j=0, mem_size=10;
	
	char *buff = (char*)malloc(mem_size);
	if (!buff)
		return NULL;
	
	while ( str[i] != '\0' ) {
		if (str[i] == '|') {
			++num_of_bar;
			if (j > 0) break;
		}
		else if (str[i] != '|' && num_of_bar >= field_to_ret) {
			buff[j++] = str[i];
			
			if (j == mem_size - 2) {
				mem_size += 10;
				char *tmp = (char*)realloc(buff, mem_size);
				if (tmp == NULL) break;
				else buff = tmp;
			}
		}
			
		i++;
	}
	buff[j]='\0';
	
	return buff;
}


struct packet *create_packet() 
{
	struct packet *np = (struct packet*)malloc(sizeof(struct packet));
	if (np == NULL)
		return NULL;
	
	np->ptype = 0;
	np->sender_id = -1;
	np->receiver_id = -1;
	np->tmsg = TXT_DATA;
	np->msg = NULL;
	
	return np;
}

void set_packet_type(struct packet *pk, msg_type type) {
	if (pk != NULL)
		pk->ptype = type;
}

int set_packet_send_id(struct packet *pk, int sender) {
	if ( pk != NULL ) {
		pk->sender_id = sender;
		return EXIT_SUCCESS;
	} else 
		return EXIT_FAILURE;
}

int set_packet_recv_id(struct packet *pk, int receiver) {
	if ( pk != NULL ) {
		pk->receiver_id = receiver;
		return EXIT_SUCCESS;
	} else 
		return EXIT_FAILURE;
}


int set_packet_msg(struct packet *pk, void *msg) {
	if (pk == NULL) {
		free(msg);
		return EXIT_FAILURE;
	}
	
	pk->msg = msg;
	return EXIT_SUCCESS;
}


char *packet_to_string(struct packet *pk) {
	return ( pk == NULL )? NULL : \
			  make_message( "|%d|%d|%d|%d|%s|",    /*packet format eg.|25|5|6|2|DATA|*/
                         pk->ptype, 
                         pk->sender_id, 
                         pk->receiver_id, 
                         pk->tmsg, 
                         (char*)pk->msg);
}

struct packet *string_to_packet(const char *str) {
	char *mem = NULL;
	
	struct packet *np = (struct packet *)malloc(sizeof(struct packet));
	if (np == NULL)
		return NULL;
	
	mem = pk_deencapsulator(str, PTYPE_FIELD);
	if (mem) {
		np->ptype = atoi(mem);
		free(mem);
	}
	
	mem = pk_deencapsulator(str, SENDER_ID_FIELD);
	if (mem) {
		np->sender_id = atoi(mem);
		free(mem);
	}
	
	mem = pk_deencapsulator(str, RECEIVER_ID_FIELD);
	if (mem) {
		np->receiver_id = atoi(mem);
		free(mem);
	}
	
	mem = pk_deencapsulator(str, MSGTYPE_FIELD);
	if (mem) {
		np->tmsg = atoi(mem);
		free(mem);
	}
	
	mem = pk_deencapsulator(str, MSG_FIELD);
	if (mem) {
		np->msg = mem;
	}
	
	return np;
}

void destroy_packet(struct packet *pk) {
	free(pk->msg);
	free(pk);
}


