#include "session.h"
#include "thr_act.h"

extern int thr_beforeAuth(char *dat, type_session *c, void *conn, int udpsock) {
    return 0;
}

extern int main_act(char *dat, short pLen, type_session *c, void *conn, int udpsock) {
    return 0;
}