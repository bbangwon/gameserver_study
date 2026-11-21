#pragma once

#define _G_TCP_PORT 9901
#define _G_UDP_PORT 8801

#define GAME_FRAME 	0x0F

/*map size*/
#define S_WIDTH  		32

#define MAXUSERNUM 	0x1000

enum {
   PK_USER_AUTH,
   PK_USER_CHAT=10,PK_USER_INFO,
   PK_REQ_LOC=20,
   PK_CMD_MV=30,PK_USER_MV, PK_USER_DISCON
};

typedef enum { 
    RET_TRUE, RET_FALSE 
} type_tf;


#define MAX_PACKET_SIZE 0x400

#define LISTEN_QUEUE 0x0A

#define AFRAME	900000/GAME_FRAME
#define _ONE_SEC	GAME_FRAME

/*packet end marker*/
#define PEND 0xC8D1

extern pthread_mutex_t 		synclock;
extern pthread_cond_t		synccond;
extern pthread_mutex_t      sendQuelock;
extern pthread_cond_t       sendQuecond;