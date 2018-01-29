#ifndef PACKETS_H
#define PACKETS_H

#include "../error_logs/errno_logs.h"

enum msg_type 
{
	TXT,
	AUDIO,
	VIDEO
};

#define PTYPE_FIELD 1
#define SENDER_ID_FIELD 2 
#define RECEIVER_ID_FIELD 3
#define MSGTYPE_FIELD 4
#define MSG_FIELD 5


struct packet {
	unsigned int ptype;
	unsigned int sender_id;
	unsigned int receiver_id;
	
	enum msg_type tmsg;
	void *msg;
};

char *pk_deencapsulator(const char *str, int field_to_ret);				

struct packet *create_packet();

void set_packet_type(struct packet *pk, unsigned type);
int set_packet_send_id(struct packet *pk, int sender);
int set_packet_recv_id(struct packet *pk, int receiver);

int set_packet_msg(struct packet *pk, void *msg);

const char *packet_to_string(struct packet *pk);
const struct packet *string_to_packet(const char *str);

void destroy_packet(struct packet *pk);

#endif
