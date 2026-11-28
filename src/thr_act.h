#pragma once

int thr_beforeAuth(char *dat, type_session *c, void *conn, int udpsock);
int main_act(char *dat, short pLen, type_session *c, void *conn, int udpsock);

void thr_party_info(type_session * c);
void thr_party_discon(type_session * c,char flag);
void thr_map_move_key_ok(char *dat, void *conn);
