#pragma once
#include "session.h"

void db_load_itemList(void *con);
void db_load_npcList(void *con);
void db_load_expList(void *con);
void db_load_skillList(void *con);

void map_usersend_All(unsigned short mapSizeX, unsigned short mapSizeZ, char *retdata, int len, type_session *c);
int db_user_auth(char *userid, char *passwd, int passlen, void *conn, type_session *c);