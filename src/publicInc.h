#pragma once
#include <pthread.h>

#define GAME_FRAME 	0x0F

#define MAXUSERNUM 	0x1000

#define _MAX_DEFAULT_ITEM 100

#define MAXUSERLEVEL 0x0A

#define MAX_PACKET_SIZE 0x400

#define _MAX_ITEM_IDX 0x0FF6

#define _MAX_MONSTER_NO 21
#define _NPC_TGT_SIG 	8

#define _MAX_ITEM_DUP 10
#define _ITEM_DUP_NO_RANGE 50

enum {T_ACTIVE, T_PASSIVE ,T_GROUP};
#define NPC_MV_FRAME		0x0D


/*packet end marker*/
#define PEND 0xC8D1

#define M_SIZE_X 512

#define S_WIDTH  		32
#define S_UNITSIZE	16

enum {
   PK_USER_AUTH,
   PK_USER_CHAT=10,PK_USER_INFO,
   PK_REQ_LOC=20,
   PK_CMD_MV=30,PK_USER_MV, PK_USER_DISCON,
   
   PK_OBJ_ADD=40, 
   PK_OBJ_REMOVE,
   PK_OBJ_UPDATE_EQUI,
   
   PK_INVEN_INFO, 
   PK_SKILL_INFO,
   PK_OBJ_UPDATE_ACT, 
   PK_ATTACK,
   PK_LEVELUP,
   PK_ITEM_DROP,
   PK_ITEM_PICK,
   PK_ITEM_POS,
   PK_ITEM_WEAR,
   PK_ITEM_BUY,
   PK_ITEM_SELL,
   PK_WARE_DEPOS,
   PK_WARE_POS,
   PK_TRADE_REQ,
   PK_TRADE_REQ_OK,
   PK_TRADE_LIST,
   PK_TRADE_SUCC,
   PK_TRADE_OK,
   PK_TRADE_CANCEL,
   PK_SKILL_GET,
   PK_SKILL_LVUP
};

/*obj type
0:USER	1:NPC/NONSTER	2:ITEM
*/
enum {T_USER,	T_NPC,	T_ITEM};

/*obj status
0:stand	1:move	2:fight	3:trade
*/
enum {S_STAND,	S_MOVE,	S_FIGHT,	S_TRADE, S_DIE, S_RET, S_REGEN, S_REMOVE};



extern pthread_mutex_t 		synclock;
extern pthread_cond_t		synccond;
extern pthread_mutex_t 		npclock;
extern pthread_mutex_t 		itemlock;

#define AFRAME	900000/GAME_FRAME
#define _ONE_SEC	GAME_FRAME
#define _TWO_MIN	GAME_FRAME*120