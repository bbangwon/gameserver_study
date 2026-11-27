#include <stdio.h>
#include <string.h>

#include "session.h"
#include "map.h"
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

extern int session_set_userid(type_session *c, char *msg, int len) {
    if(len>20) return 0;
    memcpy(&c->userid[0], &msg[0], len);
    c->userid[len] = '\0';
    c->uLen = len;
    return 1;
}

extern short session_attack_chr(type_session * c,unsigned short Index,void * conn) {
    int rLen=2;
    short result=0, wasLVup=0;
    short DaMaGe;
    short exp=0;
    char mesg[32];
    unsigned short nEnd = PEND;

    if(Index>MAXUSERNUM) return 0;
    if(sess_array[Index].flag == 0) return 0; //없는 유저
    if(sess_array[Index].s.userStat==S_DIE) return 0; //죽은 유저

    pthread_mutex_lock(&synclock);
    DaMaGe = c->attack-sess_array[Index].s.defense; //데미지 기본 공식 기본 물리공격

    if(DaMaGe<=0) DaMaGe = 1;

    sess_array[Index].s.hp_c=sess_array[Index].s.hp_c-DaMaGe;

    if(sess_array[Index].s.hp_c<=0) {
        result = 1;
        sess_array[Index].s.hp_c = 0;
        sess_array[Index].s.userStat = S_DIE; //죽은 상태로 변경
        c->userStat = S_STAND;
        c->attack_F_cnt = 0;
    }
    pthread_mutex_unlock(&synclock);

    mesg[rLen] = PK_OBJ_UPDATE_ACT;
    rLen+=1;
    mesg[rLen] = T_USER;
    rLen+=1;
    mesg[rLen] = 1;
    rLen+=1;
    memcpy(&mesg[rLen], &c->userNo, 2);
    rLen+=2;
    mesg[rLen] = S_FIGHT;
    rLen+=1;
    mesg[rLen] = c->Dir;
    rLen+=1;
    mesg[rLen]=T_USER;
    rLen+=1;
    memcpy(&mesg[rLen], &Index, 2);
    rLen+=2;
    memcpy(&mesg[rLen], &sess_array[Index].s.hp_c, 2);
    rLen+=2;
    memcpy(&mesg[rLen], &nEnd, 2);
    rLen+=2;
    memcpy(&mesg[0], &rLen, 2);
    map_usersend_All(c->Ax, c->Ay, mesg, rLen, NULL);

    return 1;

}

extern type_session * session_ret_session(unsigned short idx){
    return NULL;
}