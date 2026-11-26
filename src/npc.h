#pragma once
#include "objlist.h"
#include "session.h"


typedef struct npc_default {
    unsigned short no;
    unsigned short gtype;
    unsigned short level;
    unsigned short speed;

    unsigned short item[12];
    unsigned short rate[12];
} type_gnpc;

typedef struct npc_baseinfo
{
    char					div;
    unsigned short npc_id;

	unsigned short x;
	unsigned short y;
	unsigned short dx;
	unsigned short dy;
    unsigned short		speed;

    char					DirLoc;  
    unsigned short		ARx;  
    unsigned short		ARy;

    unsigned short		Gx;
    unsigned short		Gy;

    unsigned short status;
    unsigned short		mvstatus;

    char					tgFlag;

    unsigned short		move_F_cnt;
    unsigned short		rotate_F_cnt;
    unsigned short		ret_F_cnt;


} type_npc;

typedef struct npcmapsys
{
	type_objlist *	objL;
}
type_npcmap;

void npc_set_def(char div, short no, char gtype, char level, char speed);
void npc_set(char div, short no, char x, char y);
void npc_mv_set(unsigned short idx);
int npc_mv_one(type_npc * npc);
void npc_mv_addinfo(type_npc * npc);

void npc_3x3_sectorinfo(type_session *c);
void npc_chr_mv_addinfo(type_session *c);

void * Thread_npc_sync(void *arg);
