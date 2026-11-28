#pragma once
#include "objlist.h"
#include "session.h"

typedef struct mapsys
{
	type_objlist *	objL;
} type_map;

void map_Load();
void mapinit(type_session *c);
void map_pData_snd(type_session *c, char *mesg, int pLen);
void map_secdata_snd(short ax, short ay, char *retdata, short dLen);
void map_secdata_snd_client(short x, short z, type_session *c);
void map_usersend_All(unsigned short mapSizeX, unsigned short mapSizeZ, char *retdata, int len, type_session *c);
void map_get_sectorinfo_All(type_session *c);
short map_isgo(unsigned short x, unsigned short y);
void map_chrAction_calc(type_session * c);
int map_char_one_mv(type_session * c,unsigned short x,unsigned short y);
void map_bk_sectorinfo(type_session *c);
void map_get_sectorinfo(type_session *c);
short map_ret_dist(unsigned short x, unsigned short y, unsigned short xx, unsigned short yy);
short map_ret_chrdist(unsigned short x, unsigned short y,unsigned short idx);
void * map_ret_Activ_tg(unsigned short npcx, unsigned short npcy,unsigned short sight);
type_session * map_ret_chr(unsigned short Ax,unsigned short Ay,unsigned short tidx);
void map_party_sendAll(void * chrr,char * data,int len);