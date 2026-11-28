#pragma once
#include "session.h"

typedef struct sockaddr_in	type_sockadr;

void db_load_itemList(void *con);
void db_load_npcList(void *con);
void db_load_expList(void *con);
void db_load_skillList(void *con);

int db_user_auth(char *userid, char *passwd, int passlen, void *conn, type_session *c);
int db_chk_lvup(void * cc,void * con);

type_sockadr * db_ret_udpsock(int idx);
int db_ret_udpLen(int idx);