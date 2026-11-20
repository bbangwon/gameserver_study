#pragma once

#include "session.h"
#include "objlist.h"

 // 2차원 배열, 맵의 섹터 구조를 나타냄
 typedef struct mapsys {
    type_objlist *objL;
 } type_map;

void *voidlist_get_first(type_objlist const * const **save, unsigned short i, unsigned short j);
void *voidlist_get_next(type_objlist const * const **save);
void mapinit(type_session *c);
void map_char_rm(type_session *c);
int map_char_one_mv(type_session *c, unsigned short x, unsigned short z);
void map_bk_sectorinfo(type_session *c);
void map_get_sectorinfo(type_session *c);
void map_usersend_All(unsigned short mapSizeX, unsigned short mapSizeZ, char *retdata, int len, type_session *c);
void map_secdata_snd(unsigned short x, unsigned short z, char *udata, int dLen);