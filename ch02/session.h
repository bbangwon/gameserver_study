#pragma once

typedef enum
{
    conn_state_empty,
    conn_state_authed,
    conn_state_game,
    conn_state_destroy
} type_conn_state;
 
typedef struct connection {
    char userid[32]; //유저 아이디
    char char_name[32]; //캐릭터 이름
    unsigned short uLen; //유저 아이디 길이
    unsigned short nLen; //캐릭터 이름 길이
    unsigned short userNo; //접속 시 생성되는 캐릭터 아이디 번호
    int sock; //생성된 소켓 번호
    unsigned int addr; //클라이언트 주소
    unsigned int sessionkey; //채팅 서버에서는 사용하지 않음-서버 이동 시 비밀키
    unsigned int clientver; //게임 버전
    int disco;
    unsigned short port; //포트 번호(채팅 서버에서는 따로 사용 안 함)
    short frameCnt; //프레임 카운트
    short userStat; //유저 상태
    short roomNo; // default:0                                                              
    short Ax; //현재 위치 x좌표
    short Az; //현재 위치 z좌표
    short Bx; //이전 위치 x좌표
    short Bz; //이전 위치 z좌표
    type_conn_state state; //클라이언트 상태
} type_session;

 typedef struct connArray {
    type_session s;
    int sockNo;
    char flag; //0: not use, 1: use
 } t_sessionArray;

void init_t_sessionArray();
void reset_sessionArray(int idx);

type_session* ret_session(int idx);
type_session* ret_session_first(unsigned int max);
type_session* ret_session_next(unsigned int max);
type_session* session_create(int sock, unsigned int addr, unsigned short port);
void session_clear(type_session  * c);
int session_set_userid(type_session *c, char *msg, int len);