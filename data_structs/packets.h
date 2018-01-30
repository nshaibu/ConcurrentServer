#ifndef PACKETS_H
#define PACKETS_H

#include "../error_logs/errno_logs.h"

/*The enumerator to specify the message type*/
typedef enum {
	TXT_DATA,         /*plain text*/
	FILE_TXT_DATA,    /*plain text file .txt*/
	DOC_TEXT_DATA,    /*microsoft word docx*/
	PPT_TEXT_DATA,    /*microsoft powerpoint*/
	
	MP3_AUDIO_DATA,   /*audio mp3*/
	MP4_AUDIO_DATA,   /*audio mp4*/
	
	MP4_VIDEO_DATA,   /*video mp4*/
	MKV_VIDEO_DATA,   /*video mkv*/
	
	JPG_PIC_DATA,     /*picture jpg*/
	PNG_PIC_DATA,     /*picture png*/
	
	COORD_DATA        /*coordinate data [format data1:data2]*/
} msg_type;

//Packet types 
#define NEG_PACKET 24    /*negotiation and authentication packet*/
#define GEO_PACKET 25    /*geolocation packet*/
#define MSG_PACKET 26    /*message packet for both text and media files*/
#define ACK_PACKET 27    /*acknowledge packet*/


//field specifiers for deencapsulator
#define PTYPE_FIELD 1
#define SENDER_ID_FIELD 2 
#define RECEIVER_ID_FIELD 3
#define MSGTYPE_FIELD 4
#define MSG_FIELD 5


struct packet {
	unsigned ptype;      /*what packet is this packet?*/
	int sender_id;       /*The sender user id*/
	int receiver_id;     /*Receiver user id*/
	
	msg_type tmsg;       /*what type of data is msg?*/
	void *msg;           /*container for all messages*/
};

char *pk_deencapsulator(const char *str, int field_to_ret);    /*DeEncapsulates packets*/			

struct packet *create_packet();

void set_packet_type(struct packet *pk, unsigned type);

int set_packet_send_id(struct packet *pk, int sender);

int set_packet_recv_id(struct packet *pk, int receiver);

int set_packet_msg(struct packet *pk, void *msg);

char *packet_to_string(struct packet *pk);

struct packet *string_to_packet(const char *str);

void destroy_packet(struct packet *pk);

#endif
