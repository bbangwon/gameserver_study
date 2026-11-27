#pragma once
#include "session.h"

void db_load_itemList(void *con);
void db_load_npcList(void *con);
void db_load_expList(void *con);
void db_load_skillList(void *con);

int db_user_auth(char *userid, char *passwd, int passlen, void *conn, type_session *c);
int db_chk_lvup(void * cc,void * con);