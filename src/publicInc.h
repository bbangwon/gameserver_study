#pragma once
#include <pthread.h>

#define MAXUSERNUM 	0x1000

#define _MAX_DEFAULT_ITEM 100

#define MAXUSERLEVEL 0x0A

#define MAX_PACKET_SIZE 0x400

/*packet end marker*/
#define PEND 0xC8D1

#define S_WIDTH  		32
#define S_UNITSIZE	16

enum {
   PK_USER_AUTH,
   PK_USER_CHAT=10,PK_USER_INFO,
   PK_REQ_LOC=20,
   PK_CMD_MV=30,PK_USER_MV, PK_USER_DISCON,
   PK_OBJ_ADD=40
};

/*obj status
0:stand	1:move	2:fight	3:trade
*/
enum {S_STAND,	S_MOVE,	S_FIGHT,	S_TRADE, S_DIE, S_RET, S_REGEN, S_REMOVE};

extern pthread_mutex_t 		synclock;