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

#if !defined ( MAX_PACKET_SIZE ) || !defined ( MAX_DATA_SIZE )
#define MAX_PACKET_SIZE 200
#define MAX_DATA_SIZE MAX_PACKET_SIZE - 21
#endif

//Packet types 
#define REG_PACKET 9      /*User registration packet*/
#define AUTH_PACKET 10    /*authentication packet*/
#define GET_USERS_PACKET 11   /*ask the server for all  the users on the system and the userid*/

#define GEO_PACKET 20        /*packet received is geolocation packet[send geolocation info to self or other client]*/
#define GET_GEO_PACKET 21    /*get geolocation packet [request]*/
#define SET_GEO_PACKET 22    /*set geolocation packet [request]*/

#define MSG_PACKET 30   /*normal messages like plain text*/
#define FILE_PACKET 31  /*send messages in a form of files like .docx,.txt,.mp3*/
#define ACK_PACKET 40    /*acknowledge packet*/
#define SYN_PACKET 41    /*synchronization packet*/
#define FIN_PACKET 42    /*Finishing packet*/

#ifdef _GNUC_
#define MALLOC __attribute__(( malloc ))
#else
#define MALLOC 
#endif

struct packet {
	unsigned ptype;      /*what packet is this packet?*/
	int sender_id;       /*The sender user id*/
	int receiver_id;     /*Receiver user id*/
	
	msg_type tmsg;       /*what type of data is msg?*/
	void *msg;           /*container for all messages*/
};


struct packet *create_packet() MALLOC;

void set_packet_type(struct packet *pk, unsigned type);

int set_packet_send_id(struct packet *pk, int sender);

int set_packet_recv_id(struct packet *pk, int receiver);

int set_packet_msg(struct packet *pk, void *msg);

char *packet_to_string(struct packet *pk) MALLOC;

struct packet *string_to_packet(const char *str) MALLOC;

void destroy_packet(struct packet *pk);

#endif
