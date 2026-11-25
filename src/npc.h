#pragma once
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
    unsigned short npc_id;

    short Ax;
    short Ay;


} type_npc;

void npc_set_def(char div, short no, char gtype, char level, char speed);
void npc_set(char div, short no, char x, char y);
void npc_3x3_sectorinfo(type_session *c);
