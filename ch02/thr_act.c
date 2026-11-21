#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "packet.h"
#include "thr_act.h"
#include "map.h"
#include "db.h"
#include "publicInc.h"

#define GAME_FRAME 	0x0F
#define _ONE_SEC	GAME_FRAME
#define AFRAME	900000/GAME_FRAME

int svr_ret_tot() {
    return 0;
}

void conn_set_userid(type_session *c, char *name, int ulen) {
}

// 유저의 움직임, 자동 저장, 체력 회보ㄱ, 공격 시간 텀등 자동적으로 적용되는 캐릭터의 카운트와 관련된 일을 처리한다.
void * Thread_sync(void *arg) {
    struct timeval start, stop;
    long tmp, utmp;
    short regenTime, Half_sec;
    int i, maxNo;

    sleep(1);

    regenTime = _ONE_SEC*30; //리젠 타임
    Half_sec = _ONE_SEC/2;
    for(;;) {
        gettimeofday(&start, NULL); //시작 시간
        pthread_mutex_lock(&synclock);
        maxNo = svr_ret_tot(); //총 접속자수 
        for(i=0;i<maxNo;i++){

        }
        pthread_cond_signal(&synccond);
        pthread_mutex_unlock(&synclock);

        gettimeofday(&stop, NULL); //종료 시간
        tmp = stop.tv_sec - start.tv_sec;
        utmp = stop.tv_usec - start.tv_usec;
        //프레임에 걸리는 시간보다 작을 경우 남은 시간만큼 멈춤
        if(tmp>1) { //1frame 0.033초 33000, 1

        } else if(tmp==1) {
            utmp = 1000000 + utmp; // 걸린 시간
            if(utmp<AFRAME){
                stop.tv_sec = 0;
                stop.tv_usec = AFRAME-utmp;
                select(0, NULL, NULL, NULL, &stop); //남은 시간만큼 멈춤
            }
        } else {
            if(utmp<AFRAME) {
                stop.tv_sec = 0;
                stop.tv_usec = AFRAME - utmp;
                select(0, NULL, NULL, NULL, &stop); //남은 시간만큼 멈춤
            }
        }
    } //for 루프 끝
}

// 인증 전 상태에서 처리되는 스레드 함수
extern int thr_beforeAuth(char *dat, type_session *c, void *conn, int udpsock) {
    int pLen=2, headercd, clen, idx, ulen;
    short res;
    char name[32], pass[32];
    char mesg[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;

    memcpy(&res, &dat[0], 2); //받은 패킷의 총 길이
    res -= 2;
    headercd = dat[pLen];

    if(c->userStat == 0) { //로비 상태의 패킷 처리
        switch(headercd) {
            case PK_USER_AUTH: //헤더가 인증 요청 패킷일 경우
                idx=3;
                ulen=dat[idx]; //name 길이
                idx+=1;
                if(res<ulen+idx||ulen>31)   //오류
                    return 0;
                memcpy(&name[0], &dat[idx], ulen);    //패킷에서 유저 아이디 파싱
                name[ulen] = '\0';
                idx+=ulen;
                clen=dat[idx]; //password 길이
                idx+=1;
                if(res<clen+idx||clen>31)
                    return 0;
                memcpy(&pass[0], &dat[idx], clen);    //패킷에서 비밀번호 파싱
                pass[clen] = '\0';
                res = db_user_auth(name, pass, clen, conn); //데이터베이스 인증
                printf("유저인증 결과 : %d %s\n", res, name);

                if(res == 1) { //인증 성공
                    conn_set_userid(c, name, ulen); //유저 아이디 저장
                    mapinit(c); //맵에 초기화
                    c->state = conn_state_authed; //인증 상태로 변경
                    //map_sec_test();   //테스트
                    mesg[pLen] = PK_USER_AUTH; //응답 패킷 헤더 설정
                    pLen += 1;

                    mesg[pLen] = svr_ret_tot();
                    pLen += 1;

                    mesg[pLen]=c->uLen;
                    pLen += 1;
                    memcpy(&mesg[pLen], &c->userid[0], c->uLen);
                    pLen += c->uLen;

                    memcpy(&mesg[pLen], &c->userNo, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &nEnd, 2);
                    pLen += 2;
                    
                    memcpy(&mesg[0], &pLen, 2);
                    map_pData_snd(c, mesg, pLen);  //클라이언트에 응답 패킷 전송

                    pLen=2;
                    mesg[pLen] = PK_USER_INFO;
                    pLen+=1;

                    memcpy(&mesg[pLen], &c->userNo, 2);
                    pLen+=2;
                    
                    mesg[pLen] = c->uLen; //6
                    pLen+=1;
                    memcpy(&mesg[pLen], &c->userid[0], c->uLen); //8
                    pLen+=c->uLen;

                    mesg[pLen] = c->Ax;
                    pLen+=1;

                    mesg[pLen] = c->Az;
                    pLen+=1;

                    memcpy(&mesg[pLen], &nEnd, 2);
                    pLen+=2;

                    memcpy(&mesg[0], &pLen, 2);
                    map_usersend_All(c->Ax, c->Az, mesg, pLen, NULL);  //근처 유저에게 전송
                } else {
                    return 0;
                }
                break;
        }
    }
    return 1;
}

// 메인 액션 처리 함수
extern int main_act(char *dat, short pLen, type_session *c, void *conn, int udpsock) {
    int rLen=2, res;
    char mesg[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;
    unsigned short Ax, Az, headercd;
    short tx,tz,cLen;

    // if(dat[2] < 0) headercd=dat[2]+256; // 음수일 경우 256을 더해 양수로 변환
    headercd =dat[2];
    switch(headercd) {
        case PK_USER_CHAT:
            memcpy(&cLen, &dat[3], 2);
            if(cLen>pLen-4)
                return 0;

            mesg[rLen] = PK_USER_CHAT;
            rLen+=1;

            mesg[rLen]=c->uLen;
            rLen+=1;
            memcpy(&mesg[rLen], &c->userid[0], c->uLen);
            rLen+=c->uLen;

            memcpy(&mesg[rLen], &cLen, 2);
            rLen+=2;
            memcpy(&mesg[rLen], &dat[5], cLen);
            rLen+=cLen;

            memcpy(&mesg[rLen], &nEnd, 2);
            rLen+=2;

            memcpy(&mesg[0], &rLen, 2);
            map_usersend_All(c->Ax, c->Az, mesg, rLen, NULL);
            printf("PK_USER_CHAT:ok\n");
            break;
        case PK_REQ_LOC:
            mesg[rLen] = PK_REQ_LOC;
            rLen+=1;

            mesg[rLen] = c->Ax;
            rLen+=1;

            mesg[rLen] = c->Az;
            rLen+=1;

            memcpy(&mesg[rLen], &nEnd, 2);
            rLen+=2;

            memcpy(&mesg[0], &rLen, 2);
            map_pData_snd(c, mesg, rLen);
            printf("PK_REQ_LOC:ok\n");
            break;
        case PK_CMD_MV: //섹터 이동 패킷
            Ax=dat[3];
            Az=dat[4];
            if(Ax>=S_WIDTH||Az>=S_WIDTH)
                return 0;
            tx = c->Ax - Ax;
            tz = c->Az - Az;
            if(tx>1||tx<-1||tz>1||tz<-1||(c->Ax==Ax && c->Az==Az))
                res = 0;
            else
                res = 1;
            printf("before:%d %d after:%d %d res:%d\n", c->Ax, c->Az, Ax, Az, res);

            if(res==1){
                printf("PK_CMD_MV:TRUE\n");
                mesg[rLen] = PK_USER_MV; //2
                rLen+=1;

                mesg[rLen] = RET_TRUE;
                rLen+=1;

                memcpy(&mesg[rLen], &c->userNo, 2); // 8
                rLen+=2;

                mesg[rLen] = c->uLen;
                rLen+=1;

                memcpy(&mesg[rLen], &c->userid[0], c->uLen);
                rLen+=c->uLen;

                mesg[rLen] = Ax;
                rLen+=1;

                mesg[rLen] = Az;
                rLen+=1;

                mesg[rLen] = c->Ax;
                rLen+=1;

                mesg[rLen] = c->Az;
                rLen+=1;

                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;

                memcpy(&mesg[0], &rLen, 2);
                map_usersend_All(c->Ax, c->Az, mesg, rLen, NULL);
                map_char_one_mv(c,Ax,Az); //섹터 이동

                c->Bx = c->Ax;
                c->Bz = c->Az;
            } else {
                mesg[rLen] = PK_USER_MV;
                rLen+=1;

                mesg[rLen] = RET_FALSE;
                rLen+=1;

                memcpy(&mesg[rLen], &c->userNo, 2);
                rLen+=2;

                mesg[rLen] = c->uLen;
                rLen+=1;

                memcpy(&mesg[rLen], &c->userid[0], c->uLen);
                rLen+=c->uLen;

                mesg[rLen] = c->Ax;
                rLen+=1;

                mesg[rLen] = c->Az;
                rLen+=1;

                mesg[rLen] = Ax;
                rLen+=1;

                mesg[rLen] = Az;
                rLen+=1;

                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;
                memcpy(&mesg[0], &rLen, 2);
                map_usersend_All(c->Ax, c->Az, mesg, rLen, NULL);
            }
            break;
        default:
            return 0;
    }
    return 1;
}