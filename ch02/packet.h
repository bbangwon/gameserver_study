 #pragma once
 
 #define MAX_PACKET_SIZE 0x400

 /*packet end marker*/
#define PEND 0xC8D1

enum {
   PK_USER_AUTH,
   PK_USER_CHAT=10,PK_USER_INFO,
   PK_REQ_LOC=20,
   PK_CMD_MV=30,PK_USER_MV,
   PK_USER_DISCON=90
};

 //클라이언트에 보내질 패킷의 형
 typedef struct packet {
    int cli_sock;
    int size;
    char data[MAX_PACKET_SIZE];
    void *session;
 } type_packet;

 type_packet *packet_create();
 void packet_destroy(type_packet *packet);