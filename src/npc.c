#include "npc.h"

static type_gnpc def_mon[21]; //몬스터 정보 리스트
static type_gnpc def_npc[1]; //NPC 정보 리스트

extern void npc_set_def(char div, short no, char gtype, char level, char speed)
{
    if(div==0) { // NPC일 경우
        def_npc[no].no = no;
        def_npc[no].gtype = gtype;
        def_npc[no].level = level;
        def_npc[no].speed = speed;
    } else { // 몬스터일 경우
        def_mon[no].no = no;
        def_mon[no].gtype = gtype;
        def_mon[no].level = level;
        def_mon[no].speed = speed;
    }
}

extern void npc_set(char div, short no, char x, char y)
{
    
}

extern void npc_3x3_sectorinfo(type_session *c) {
}

extern void npc_chr_mv_addinfo(type_session *c) {
}