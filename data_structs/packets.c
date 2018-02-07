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
                         pk->ptype,             /*if the header is having 4 chars for */
                         pk->sender_id,         /* each field then the max size of the */
                         pk->receiver_id,       /* header is 21 chars i.e /0000/0000/0000/0000/.*/
                         pk->tmsg,              /* So the rest belongs to the DATA field. Thus*/
                         (char*)pk->msg);       /* the max data size is 200-21 = 179chars */
}

struct packet *string_to_packet(const char *str) {
	char *mem, *word;
	char tmp[11];
	char *str_use = strdup(str);
	
	struct packet *np = (struct packet *)malloc(sizeof(struct packet));
	if (np == NULL)
		return NULL;
	
	word = strtok_r(str_use, "|", &mem);
	if ( word != NULL) {
		strncpy(tmp, word, 10);
		np->ptype = atoi(word);
	}
	
	word = strtok_r(NULL, "|", &mem);
	if ( word != NULL ) {
		strncpy(tmp, word, 10);
		np->sender_id = atoi(word);
	}
	
	word = strtok_r(NULL, "|", &mem);
	if ( word != NULL ) {
		strncpy(tmp, word, 10);
		np->receiver_id = atoi(word);
	}
	
	word = strtok_r(NULL, "|", &mem);
	if ( word != NULL ) {
		strncpy(tmp, word, 10);
		np->tmsg = atoi(word);
	}
	
	word = strtok_r(NULL, "|", &mem);
	if ( word != NULL ) {
		np->msg = strndup(word, MAX_DATA_SIZE);
	}
	
	return np;
}

void destroy_packet(struct packet *pk) {
	void *hold = pk->msg;
	free(hold);
	pk->msg = NULL;
	free(pk);
	pk=NULL;
}


