 #pragma once
 
 #define MAX_PACKET_SIZE 0x400

 //클라이언트에 보내질 패킷의 형
 typedef struct packet {
    int cli_sock;
    int size;
    char data[MAX_PACKET_SIZE];
    void *session;
 } type_packet;