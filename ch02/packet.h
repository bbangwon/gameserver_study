 #pragma once

 #include "publicInc.h"
 
  //클라이언트에 보내질 패킷의 형
 typedef struct packet {
    int cli_sock;
    int size;
    char data[MAX_PACKET_SIZE];
    void *session;
 } type_packet;

 type_packet *packet_create();
 void packet_destroy(type_packet *packet);