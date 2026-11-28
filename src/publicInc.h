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
   PK_SKILL_LVUP,
   PK_SKILL_USE,
   PK_PT_REQ,
   PK_PT_PERMIT,
   PK_PT_INFO,
   PK_PT_BAN,
   PK_PT_BREAK,
   PK_PT_LEAVE,

   PK_MAP_MOVE_REQ,

   PKU_MAP_KEY
};

/*obj type
0:USER	1:NPC/NONSTER	2:ITEM
*/
enum {T_USER,	T_NPC,	T_ITEM};

/*obj status
0:stand	1:move	2:fight	3:trade
*/
enum {S_STAND,	S_MOVE,	S_FIGHT,	S_TRADE, S_DIE, S_RET, S_REGEN, S_REMOVE};


/*return value settings*/
typedef enum {
	RET_TRUE, RET_FALSE, RET_REDUN, RET_NOID, RET_FULL, RET_NOTENO
}type_tf;


extern pthread_mutex_t 		synclock;
extern pthread_cond_t		synccond;
extern pthread_mutex_t 		npclock;
extern pthread_mutex_t 		itemlock;

#define AFRAME	900000/GAME_FRAME
#define _ONE_SEC	GAME_FRAME
#define _TWO_MIN	GAME_FRAME*120

#define _MAP_IDX 0   //서버 자신의 인덱스

#define _MAP_GWAY_0_X 250 //0번 이동 통로의 X좌표
#define _MAP_GWAY_0_Y 5 //0번 이동 통로의 Y좌표
#define _MAP_GWAY_0_TO_SVR 1 //0번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_1_X 506 //1번 이동 통로의 X좌표
#define _MAP_GWAY_1_Y 256 //1번 이동 통로의 Y좌표
#define _MAP_GWAY_1_TO_SVR 2 //1번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_2_X 0 //2번 이동 통로의 X좌표
#define _MAP_GWAY_2_Y 0 //2번 이동 통로의 Y좌표
#define _MAP_GWAY_2_TO_SVR 0 //2번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_3_X 0 //3번 이동 통로의 X좌표
#define _MAP_GWAY_3_Y 0 //3번 이동 통로의 Y좌표
#define _MAP_GWAY_3_TO_SVR _MAP_IDX //3번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_4_X 0 //4번 이동 통로의 X좌표
#define _MAP_GWAY_4_Y 0 //4번 이동 통로의 Y좌표
#define _MAP_GWAY_4_TO_SVR _MAP_IDX //4번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_5_X 0 //5번 이동 통로의 X좌표
#define _MAP_GWAY_5_Y 0 //5번 이동 통로의 Y좌표
#define _MAP_GWAY_5_TO_SVR _MAP_IDX //5번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_6_X 0 //6번 이동 통로의 X좌표
#define _MAP_GWAY_6_Y 0 //6번 이동 통로의 Y좌표
#define _MAP_GWAY_6_TO_SVR _MAP_IDX //6번 이동 통로로 이어지는 월드 서버 인덱스

#define _MAP_GWAY_7_X 0 //7번 이동 통로의 X좌표 
#define _MAP_GWAY_7_Y 0 //7번 이동 통로의 Y좌표
#define _MAP_GWAY_7_TO_SVR _MAP_IDX //7번 이동 통로로 이어지는 월드 서버 인덱스

#define	SA	struct sockaddr