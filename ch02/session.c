#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include "session.h"

static t_sessionArray sess_array[MAXUSERNUM];
static volatile int sockTosess[MAXUSERNUM];

static int ar_idx;
static int ar_cnt;

void map_char_rm(type_session *c) {
    
}

//유저의 연결 정보 배열을 초기화한다.
extern void init_t_sessionArray()
{
    int i;

    for(i=0;i<MAXUSERNUM;i++){
        sess_array[i].flag=0;
        sess_array[i].sockNo=i;
    }
}

//연결 정보 배열의 해당 인덱스를 "사용 안함"으로 설정
extern void reset_sessionArray(int idx) {
    if(idx>=MAXUSERNUM || idx<0)
        return;

    sess_array[idx].flag=0;
}

// 연결 정보 배열에서 해당 연결 정보를 리턴한다.
extern type_session* ret_session(int idx) {
    if(idx>=MAXUSERNUM || idx <= 0)
        return NULL;

    return &sess_array[sockTosess[idx]].s;
}

//연결 정보 배열에서 사용 중인 첫번째 연결 정보를 리턴한다.
// max : 현재 접속 총원
extern type_session* ret_session_first(unsigned int max) {
    int i;

    if(max==0)
        return NULL;

    for(i=0;i<MAXUSERNUM;i++){
        if(sess_array[i].flag == 1){
            ar_idx = i; //다음 유저를 찾을 때 사용할 인덱스
            ar_cnt = 1; //배열 인덱스 카운트
            return &sess_array[i].s;
        }
    }
    return NULL;
}

extern type_session* ret_session_next(unsigned int max) {
    int i;

    for(i=ar_idx+1;i<MAXUSERNUM;i++){
        if(sess_array[i].flag == 1){
            ar_idx = i; //다음 유저를 찾을 때 사용할 인덱스
            ar_cnt += 1;
        }
    }
}

//유저가 접속 요청을 할 때 유저의 정보를 저장하는 역할을 한다. 미 사용중인 정보 배열을 찾아 멤버 변수를 초기화 한다.
extern type_session* session_create(int sock, unsigned int addr, unsigned short port) {
    int i;

    if(sock < 0) 
        return NULL;

    for(i=0;i<MAXUSERNUM;i++) {
        if(sess_array[i].flag == 0) {
            sess_array[i].flag=1;
            break;
        }
    }
    sess_array[i].s.sock = sock;
    sess_array[i].s.port = port;
    sess_array[i].s.disco = 0;
    sess_array[i].s.addr = addr;
    sess_array[i].s.state = conn_state_empty;
    sess_array[i].s.clientver = 0;
    sess_array[i].s.frameCnt = 0; //프레임 카운트
    sess_array[i].s.userStat = 0; //유저 상태(0: 로비, 1: 게임 방참가, 2: 게임 중, 3: 죽음)

    sess_array[i].s.roomNo = 0; //기본값 0
    sess_array[i].s.userNo = i;
    sockTosess[sock] = i;
    sess_array[i].s.Ax = 0;
    sess_array[i].s.Az = 0;
    sess_array[i].s.Bx = 0;
    sess_array[i].s.Bz = 0;

    return &sess_array[i].s;
}

// 유저와의 접속이 끊어질 때 채팅 서버에 있는 유저 정보를 초기화해준다.
extern void session_clear(type_session  * c) {
    if(!c)
        return;

    if(c->userStat != 0) {
        //thr_reset_gameroominfo(c, c->roomNo);
    }
    reset_sessionArray(c->userNo);
    map_char_rm(c);

    shutdown(c->sock, 2);
    close(c->sock);
    //free(c);
}

// 클라이언트로부터 받은 데이터로 type_session 구조체의 userid 멤버 변수를 설정한다.
extern int session_set_userid(type_session *c, char *msg, int len) {
    if(len>20)
      return 0;
    
      memcpy(&c->userid[0],&msg[0],len);
      c->userid[len] = '\0';
      c->uLen = len;
      return 1;
}







