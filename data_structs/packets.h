#ifndef PACKETS_H
#define PACKETS_H

enum msg_type {
	TXT,
	AUDIO,
	VIDEO
};


struct packet {
	unsigned int ptype;
	unsigned int sender_id;
	unsigned int receiver_id;
	
	enum msg_type tmsg;
	void *msg;
};

struct packet *create_packet(enum msg_type tm);

void set_packet_type(struct packet *pk, unsigned type);
int set_packet_send_id(struct packet *pk, unsigned sender);
int set_packet_recv_id(struct packet *pk, unsigned receiver);

int set_packet_msg(struct packet *pk, void *msg);

const char *packet_to_string(struct packet *pk);
const struct packet *string_to_packet(const char *str);

#endif
