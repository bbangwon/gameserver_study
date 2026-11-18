#pragma once

#include "session.h"

void * Thread_sync(void *arg);
int thr_beforeAuth(char *dat, type_session *c, void *conn, int udpsock);