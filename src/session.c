#include <stdio.h>
#include "session.h"
#include "publicInc.h"

static t_sessionArray sess_array[MAXUSERNUM];
static volatile int sockTosess[MAXUSERNUM];

extern type_session* ret_session_first(unsigned int max) {
    return NULL;
}

extern type_session* ret_session_next(unsigned int max) {
    return NULL;
}

extern type_session* session_create(int sock, unsigned int addr, unsigned short port) {
    int i;
    
    if(sock < 0) return NULL;
    for(i=0;i<MAXUSERNUM;i++) {
        if(sess_array[i].flag == 0) {
            sess_array[i].flag = 1;
            break;
        }
    }
    sess_array[i].s.sock = sock;
    sess_array[i].s.port = port;
    sess_array[i].s.disco = 0;
    sess_array[i].s.addr = addr;
    sess_array[i].s.state = conn_state_empty; //인증된 상태
    sess_array[i].s.clientver = 0;
    sess_array[i].s.frameCnt = 0; //프레임 카운트
    sess_array[i].s.userStat = 0; // 유지상태 0: 로비 1: 게임방참가
    sess_array[i].s.roomNo = 0; //기본정보 0
    sess_array[i].s.dbLoaded = 0;
    sess_array[i].s.userNo = 0; //유저 배열 인덱스 
    sockTosess[sock] = 1;
    sess_array[i].s.Ax = 0;
    sess_array[i].s.Ay = 0; //추가 좌표 초기화
    sess_array[i].s.Bx = 0;
    sess_array[i].s.By = 0; //추가 좌표 초기화

    return &sess_array[i].s;
}

extern void session_clear(type_session  * c) {
}