#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/tcp.h>

#include "session.h"
#include "db.h"
#include "map.h"
#include "thr_act.h"
#include "npc.h"
#include "item.h"
#include "svr.h"
#include "publicInc.h"



static short mvAction_set(type_session *c, short Dx, short Dy, short dirL){
    if(!c) return 0;
    c->Dir = dirL;
    c->userStat = 1;
    if((map_isgo(Dx,Dy))==1) return 0; //이동 가능지역 체크
    c->Dx = Dx; //목적지 셀 좌표
    c->Dy = Dy;
    return 1;
}

static void thr_inveninfo(type_session *c) {
    char msg[128];
    short Len=2;
    int i,cnt = 0;
    unsigned short nEnd = PEND;

    if(c==NULL) return;

    msg[Len] = PK_INVEN_INFO;
    Len +=2;

    for(i=0;i<4;i++) { // 인벤토리 총 수만큼 루프
        if(c->inven[i]>0) {
            msg[Len] = i;
            Len += 1;
    
            memcpy(&msg[Len], &c->inven[i], 2);
            Len += 2;
    
            msg[Len]=c->inven_cnt[i];
            Len+=1;
    
            cnt+=1;
        }
    }
    msg[3]=cnt; //인벤토리의 총 아이템 수
    memcpy(&msg[Len], &nEnd, 2);
    Len += 2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);  //클라이언트에 인벤토리 정보 전송
}

static void thr_skillinfo(type_session *c) {
    char msg[128];
    short Len=2;
    int i,cnt = 0;
    unsigned short nEnd = PEND;

    if(c==NULL) return;

    msg[Len]=PK_SKILL_INFO;
    Len += 2;

    for(i=0;i<4;i++) { // 스킬 총 수만큼 루프
        if(c->skill[i]>0) {
            msg[Len] = i;
            Len += 1;

            memcpy(&msg[Len], &c->skill[i], 2);
            Len += 2;

            msg[Len] = c->skill_lv[i];
            Len += 1;
            cnt += 1;
        }
    }
    msg[3] = cnt;
    memcpy(&msg[Len], &nEnd, 2);
    Len += 2;
    
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

extern void thr_party_info(type_session * c)
{
    type_session *mc = NULL;

    unsigned short nEnd = PEND;
    short pLen =2, res = 0;
    char msg[MAX_PACKET_SIZE];

    msg[pLen] = PK_PT_INFO;
    pLen+=1;
    msg[pLen] = res;
    pLen +=1;
    if(c->party.cnt==3) {
        mc=(type_session *)c->party.mem2;
        if(mc!=NULL) {
            memcpy(&msg[pLen], &mc->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_m, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_c, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cx, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cy, 2);
            pLen += 2;
        } else res=1;
        mc=(type_session *)c->party.mem3;
        if(mc!=NULL) {
            memcpy(&msg[pLen], &mc->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_m, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_c, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cx, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cy, 2);
            pLen += 2;
        } else res=2;
    } else if(c->party.cnt==2) {
        mc=(type_session *)c->party.mem2;
        if(mc!=NULL) {
            memcpy(&msg[pLen], &mc->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_m, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->hp_c, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cx, 2);
            pLen += 2;
            memcpy(&msg[pLen], &mc->Cy, 2);
            pLen += 2;
        } else {
            mc=(type_session *)c->party.mem3;
            if(mc==NULL){
                res=3;
            }else{
                memcpy(&msg[pLen], &mc->userNo, 2);
                pLen += 2;
                memcpy(&msg[pLen], &mc->hp_m, 2);
                pLen += 2;
                memcpy(&msg[pLen], &mc->hp_c, 2);
                pLen += 2;
                memcpy(&msg[pLen], &mc->Cx, 2);
                pLen += 2;
                memcpy(&msg[pLen], &mc->Cy, 2);
                pLen += 2;
            }
        }
    } else res=4;

    if(res==0) {
        memcpy(&msg[pLen], &c->userNo, 2);
        pLen += 2;
        memcpy(&msg[pLen], &c->hp_m, 2);
        pLen += 2;
        memcpy(&msg[pLen], &c->hp_c, 2);
        pLen += 2;
        memcpy(&msg[pLen], &c->Cx, 2);
        pLen += 2;
        memcpy(&msg[pLen], &c->Cy, 2);
        pLen += 2;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2);
        map_party_sendAll(c, msg, pLen);
    } else {
        msg[3]=res;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2);
        map_party_sendAll(c, msg, pLen);
    }
}

extern int thr_beforeAuth(char *dat, type_session *c, void *conn, int udpsock) {
    int pLen=2, headercd, clen, idx, ulen;
    short res;
    char name[32], pass[32];
    char mesg[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;
    type_session * usr;
	type_npc * npc;

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
                res = db_user_auth(name, pass, clen, conn, c); //데이터베이스 인증
                printf("유저인증 결과 : %d %s\n", res, name);

                if(res == 1) { //인증 성공
                    pthread_mutex_lock(&synclock);
                    session_set_userid(c, name, ulen); //유저 아이디 저장
                    mapinit(c); //맵에 초기화
                    c->state = conn_state_authed; //인증 상태로 변경
                    pthread_mutex_unlock(&synclock);
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

                    //myinfo sync
                    pLen=2;
                    mesg[pLen]=PK_OBJ_ADD;
                    pLen += 1;
                    mesg[pLen]=0; //객체 타입 0: 유저 1: npc 2: 아이템
                    pLen += 1;

                    mesg[pLen]=1; //카운트
                    pLen += 1;
                    memcpy(&mesg[pLen], &c->userNo, 2); //유저 번호
                    pLen += 2;
                    mesg[pLen]=c->nLen;
                    pLen += 1;
                    memcpy(&mesg[pLen], &c->char_name[0], c->nLen);
                    pLen += c->nLen;
                    mesg[pLen]=c->race;
                    pLen += 1;
                    mesg[pLen]=c->sex;
                    pLen += 1;
                    mesg[pLen]=c->nation;
                    pLen += 1;

                    memcpy(&mesg[pLen], &c->str, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->dex, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->intel, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->hp_c, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->mana_c, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->exp, 4);
                    pLen += 4;

                    memcpy(&mesg[pLen], &c->level, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->lvpoint, 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->skexp, 4);
                    pLen += 4;

                    mesg[pLen] = c->jobno;
                    pLen += 1;

                    mesg[pLen] = c->classno;
                    pLen += 1;

                    mesg[pLen] = c->classlevel;
                    pLen += 1;

                    memcpy(&mesg[pLen], &c->eq[0], 2);
                    pLen += 2;
                    memcpy(&mesg[pLen], &c->eq[1], 2);
                    pLen += 2;
                    memcpy(&mesg[pLen], &c->eq[2], 2);
                    pLen += 2;
                    memcpy(&mesg[pLen], &c->eq[3], 2);
                    pLen += 2;
                    memcpy(&mesg[pLen], &c->eq[4], 2);
                    pLen += 2;

                    memcpy(&mesg[pLen], &c->coin, 4);
                    pLen += 4;

                    mesg[pLen]=c->userStat;
                    pLen += 1;
                    if(c->userStat==S_STAND) {
                        mesg[pLen] = c->Dir;
                        pLen += 1;
                        memcpy(&mesg[pLen], &c->Ax, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Ay, 2);
                        pLen += 2;
                    } else if(c->userStat==S_MOVE) { //이동
                        mesg[pLen] = c->Dir;
                        pLen += 1;
                        mesg[pLen] = c->moveLevel;
                        pLen += 1;
                        memcpy(&mesg[pLen], &c->Cx, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Cy, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Dx, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Dy, 2);
                        pLen += 2;
                    } else if(c->userStat==S_FIGHT) { //전투
                        mesg[pLen] = c->Dir;
                        pLen += 1;
                        mesg[pLen] = c->tgtype;
                        pLen += 1;
                        if(c->tgtype==0){ //pk
                            usr=(type_session*)c->target;
                            memcpy(&mesg[pLen], &usr->userNo, 2);
                            pLen += 2;
                        } else { //몬스터

                            npc=(type_npc*)c->target;
                            memcpy(&mesg[pLen], &npc->npc_id, 2);
                            pLen += 2;
                        }
                        memcpy(&mesg[pLen], &c->Ax, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Ay, 2);
                        pLen += 2;
                    } else { //거래
                        mesg[pLen] = c->Dir;
                        pLen += 1;
                        memcpy(&mesg[pLen], &c->Ax, 2);
                        pLen += 2;
                        memcpy(&mesg[pLen], &c->Ay, 2);
                        pLen += 2;
                    }
                    memcpy(&mesg[pLen] , &nEnd, 2);
                    pLen += 2;
                    memcpy(&mesg[0], &pLen, 2);
                    map_usersend_All(c->Ax, c->Ay, mesg, pLen, NULL);  //근처 유저에게 전송

                    thr_inveninfo(c); //플래그 하나 만들어서 클라이언트가 요청할 때 보내주게 할 수도 있다.
                    thr_skillinfo(c);

                    map_get_sectorinfo_All(c); //사용자 정보 받아오기
                    npc_3x3_sectorinfo(c); //필드의 npc, 몬스터 정보 받아오기
                    item_3x3_sectorinfo(c); //필드의 아이템 정보 받아오기
                }else {
                    return 0;
                }
                    
                break;
        }
    }
    return 1;
}

extern int main_act(char *dat, short pLen, type_session *c, void *conn, int udpsock) {
    int rLen=2, res, range;
    char mesg[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;
    unsigned short headercd, idx;
    short Tx,Ty;
    unsigned char DIR;
    
    headercd =dat[2];
    switch(headercd) {
        case PK_CMD_MV: 
            memcpy(&Tx, &dat[3], 2); //이동할 셀의 좌표
            memcpy(&Ty, &dat[5], 2);
            DIR=dat[7];
            if(Tx>=M_SIZE_X||Ty>=M_SIZE_X||DIR>=8) { //전송받은 값이 올바른지 체크
                res=0;
            } else {
                if(c->Cx==Tx && c->Cy==Ty) res=0;
                else {
                    pthread_mutex_lock(&synclock);
                    res=mvAction_set(c, Tx, Ty, DIR);
                    pthread_mutex_unlock(&synclock);
                }
            }
            printf("before:%d,%d after:%d,%d res:%d\n", c->Cx, c->Cy, Tx, Ty, res);
            mesg[rLen] = PK_OBJ_UPDATE_ACT;
            rLen += 1;
            
            mesg[rLen] = T_USER;
            rLen += 1;

            mesg[rLen] = 1; //카운트
            rLen += 1;

            memcpy(&mesg[rLen], &c->userNo, 2);
            rLen += 2;

            mesg[rLen] = S_MOVE;
            rLen += 1;

            mesg[rLen]=c->Dir;
            rLen += 1;

            mesg[rLen]=c->moveLevel;
            rLen += 1;

            memcpy(&mesg[rLen], &c->Cx, 2);
            rLen += 2;
            memcpy(&mesg[rLen], &c->Cy, 2);
            rLen += 2;
            memcpy(&mesg[rLen], &c->Dx, 2);
            rLen += 2;
            memcpy(&mesg[rLen], &c->Dy, 2);
            rLen += 2;

            memcpy(&mesg[rLen], &nEnd, 2);
            rLen += 2;
            memcpy(&mesg[0], &rLen, 2); // 패킷 길이 정보 업데이트

            if(res==1){
                map_usersend_All(c->Ax, c->Ay, mesg, rLen, NULL);
            }
            break;
        case PK_ATTACK:
            if(c->userStat == S_DIE) return 0;

            if(c->eq[2]==0) range=1; //무기 없음. 오른손만 체크 무조건 공격거리 1
            else range=item_get_range(c->eq[2]);

            if(dat[3] == 0) { //유저
                memcpy(&idx, &dat[4], 2);
                res = map_ret_chrdist(c->Cx, c->Cy, idx);
            } else {
                memcpy(&idx, &dat[4], 2);
                if(idx>=_MAX_MONSTER_NO) return 0;
                res = npc_ret_dist(c->Cx, c->Cy, idx);
            }
            if(range<res||res==-1) return res;
            if(c->attack_F_cnt==-1){ //공격 가능
                if(dat[3]==0){ //유저
                    res=session_attack_chr(c,idx,conn);
                }else {
                    res=npc_chr_attack_npc(c, idx, conn);
                }
                return res;
            }else return 0;
            break;
    }
}

static void thr_party_request(type_session *c, unsigned short tgidx) {
    type_session *tc;
    unsigned short nEnd = PEND;
    short pLen=2, lvpt;
    char msg[128];

    msg[pLen] = PK_PT_REQ;
    pLen += 1;

    if(c->level<4){ //레벨이 4보다 작으면 파티장을 못함
        msg[pLen] = 1;
        pLen += 1;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);
        return;
    }

    if(c->party.flag==1) {
        if(c->party.cnt == 3 || c->party.ison1 != 2) { //파티원 초과 또는 파티장이 아님
            msg[pLen] = 2; // 파티원 초과 또는 파티장이 아님
            pLen += 1;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
            map_pData_snd(c, msg, pLen);
            return;
        }
    }

    tc=map_ret_chr(c->Ax, c->Ay, tgidx); //거리 체크 기능
    if(tc==NULL) {
        msg[pLen] = 3; // 대상 유저가 존재하지 않음
        pLen += 1;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);
        return;
    }
    if(tc->party.flag==1) { //이미 파티중
        msg[pLen] = 4; // 대상 유저가 이미 파티 중임
        pLen += 1;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);
        return;
    }
    if(tc->level>=c->level)
        lvpt = tc->level-c->level;
    else
        lvpt = c->level-tc->level;
    if(lvpt > 5) {
        msg[pLen] = 5; // 레벨 차이가 너무 큼
        pLen += 1;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);
        return;
    }

    msg[pLen] = RET_TRUE;
    pLen += 1;
    memcpy(&msg[pLen], &c->userNo, 2);
    pLen += 2;
    memcpy(&msg[pLen], &nEnd, 2);
    pLen += 2;
    memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트

    pthread_mutex_lock(&synclock);
    c->party.waitfor=tc;
    tc->party.waitfor=c;
    pthread_mutex_unlock(&synclock);

    map_pData_snd(c, msg, pLen);
}

static void thr_party_permit(type_session *c, unsigned short ptMain, short flag) {
    // ptMain: 파티장, flag 0:허락, 1: 거절 2: 다른 작업중
    type_session *tc = NULL;    //파티 요청자
    type_session *cc = NULL; //파티장이 가지고 있는 대상자
    type_session *mc = NULL; //파티원
    unsigned short nEnd = PEND;
    short pLen = 2, tNo=0, Len;
    int newFlag=0;
    char msg[512], msg2[512];
    
    msg[pLen] = PK_PT_PERMIT;
    pLen += 1;

    if(c->party.flag == 1) { //이 패킷은 파티 대상자가 보낸 것이다. 이미 파티 중임
        msg[pLen] = 3;
        pLen += 1;
        memcpy(&msg[pLen], &c->userNo, 2);
        pLen += 2;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);
        return;
    }

    tc=(type_session *)c->party.waitfor; //파티장
    if(tc!=NULL) {
        cc=(type_session *)tc->party.waitfor; //파티장이 가지고 있는 대상자
        if(cc==NULL) {
            msg[pLen] = 4; // 파티장이 이미 다른 작업 중임
            pLen += 1;
            memcpy(&msg[pLen], &c->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
            map_pData_snd(c, msg, pLen);
            return;
        }

        if(flag!=0) {
            msg[pLen] = 1; // 거절
            pLen += 1;
            memcpy(&msg[pLen], &c->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); 
            map_pData_snd(tc, msg, pLen); //파티장에게만 전송
            return;
        }

        if(tc->userNo==ptMain&&cc->userNo==c->userNo) { //파티장 찾음
            if(tc->party.flag == 1) { //이미 파티 중일 경우
                if(tc->party.ison1!=2) {
                    msg[pLen] = 5; // 이미 파티 중임
                    pLen += 1;
                    memcpy(&msg[pLen], &c->userNo, 2);
                    pLen += 2;
                    memcpy(&msg[pLen], &nEnd, 2);
                    pLen += 2;
                    memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
                    map_pData_snd(c, msg, pLen);
                    return;
                }
            }

            pthread_mutex_lock(&synclock);
            if(tc->party.flag == 0) { //첫 파티 결성
                newFlag = 1;
                tc->party.flag = 1;
                tc->party.cnt = 2;

                tc->party.memidx=1; //ptmain
                tc->party.master=NULL;
                tc->party.ison1 = 2; // 파티장 상태 설정
                tc->party.mem2 = c;
                tc->party.ison2 = 1;

                c->party.flag = 1;
                c->party.master = tc;
                c->party.ison1 = 1; 
                c->party.ison2 = 2;

                tNo=2;
            } else {
                if(tc->party.cnt == 3) {
                    pthread_mutex_unlock(&synclock);
                    msg[pLen] = 6; // 파티가 가득 찼음
                    pLen += 1;
                    memcpy(&msg[pLen], &c->userNo, 2);
                    pLen += 2;
                    memcpy(&msg[pLen], &nEnd, 2);
                    pLen += 2;
                    memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
                    map_pData_snd(c, msg, pLen);                    
                    map_pData_snd(tc, msg, pLen);                    
                    return;
                }

                newFlag=0;
                tc->party.cnt = 3;
                c->party.flag = 1;
                c->party.master = tc;
                c->party.ison1 = 1;

                if(tc->party.ison2 == 0) {
                    tNo=2;
                    mc=(type_session*)tc->party.mem3;
                    if(mc==NULL) tNo=0;
                    else {
                        tc->party.mem2 = c;
                        tc->party.ison2 = 1;
                        c->party.mem2 = c;
                        c->party.ison2 = 2;
                        c->party.memidx = 2;
                        mc->party.mem3 = c;
                        mc->party.ison3 = 1;
                    }
                } else if(tc->party.ison3 == 0) {
                    tNo=3;
                    mc=(type_session*)tc->party.mem2;
                    if(mc==NULL) tNo=0;
                    else {
                        tc->party.mem3 = c;
                        tc->party.ison3 = 1;
                        c->party.mem3 = c;
                        c->party.ison3 = 2;
                        c->party.memidx = 3;
                        mc->party.mem2 = c;
                        mc->party.ison2 = 1;
                    }
                }
                if(tNo == 0) {
                    pthread_mutex_unlock(&synclock);
                    msg[pLen] = 7;
                    pLen += 1;
                    memcpy(&msg[pLen], &c->userNo, 2);
                    pLen += 2;
                    memcpy(&msg[pLen], &nEnd, 2);
                    pLen += 2;
                    memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
                    map_pData_snd(c, msg, pLen);                    
                    map_pData_snd(tc, msg, pLen);                    
                    return;
                }
            }
            pthread_mutex_unlock(&synclock);

            msg[pLen] = RET_TRUE;
            pLen += 1;
            msg[pLen] = 2;
            pLen += 1;
            memcpy(&msg[pLen], &c->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); 

            Len=2;
            msg2[Len] = PK_PT_INFO;
            Len += 1;
            msg2[Len] = 2;
            Len += 1;
            memcpy(&msg2[Len], &c->userNo, 2);
            Len += 2;
            memcpy(&msg2[Len], &c->hp_c, 2);
            Len += 2;
            memcpy(&msg2[Len], &c->Cx, 2);
            Len += 2;
            memcpy(&msg2[Len], &c->Cy, 2);
            Len += 2;
            memcpy(&msg2[Len], &tc->userNo, 2);
            Len += 2;
            memcpy(&msg2[Len], &tc->hp_m, 2);
            Len += 2;
            memcpy(&msg2[Len], &tc->hp_c, 2);
            Len += 2;
            memcpy(&msg2[Len], &tc->Cx, 2);
            Len += 2;
            memcpy(&msg2[Len], &tc->Cy, 2);
            Len += 2;
            memcpy(&msg2[Len], &nEnd, 2);
            Len += 2;
            memcpy(&msg2[0], &Len, 2);

            if(newFlag==1){
                map_pData_snd(tc, msg, pLen); //파티장에게 전송
                map_pData_snd(c, msg2, Len); //새 파티원에게 전송
            } else {
                map_pData_snd(tc, msg, pLen); //파티장에게 전송
                map_pData_snd(mc, msg, pLen); //기존 파티원에게 전송
                map_pData_snd(c, msg2, Len); //새 파티원에게 전송
            }
            return;
        } else {
            msg[pLen] = 7;
            pLen += 1;
            memcpy(&msg[pLen], &c->userNo, 2);
            pLen += 2;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
            map_pData_snd(c, msg, pLen);
            return;
        }        
    } else {
        msg[pLen] = 8;
        pLen += 1;
        memcpy(&msg[pLen], &c->userNo, 2);
        pLen += 2;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
        map_pData_snd(c, msg, pLen);        
    }
}

static void thr_party_set(type_session *c){

}

//파티원 추방
static void thr_part_bann(type_session *c, char *dat) {
    type_session *mc1 = NULL;
    type_session *mc2 = NULL;

    unsigned short nEnd = PEND;
    short pLen = 2;
    unsigned short idx;
    char msg[128];

    if(c==NULL) return;
    if(c->party.flag == 0||c->party.ison1!= 2) return;
    memcpy(&idx, &dat[3], 2);

    if(c->party.cnt == 2) {
        if(c->party.ison2 == 1) mc1=(type_session *)c->party.mem2;
        if(mc1!=NULL) {
            pthread_mutex_lock(&synclock);
            thr_party_set(mc1);
            thr_party_set(c);
            pthread_mutex_unlock(&synclock);
            msg[pLen] = PK_PT_BAN;
            pLen+=1;
            msg[pLen]=1;
            pLen += 1;
            memcpy(&msg[pLen], &idx, 2);
            pLen += 2;
            memcpy(&msg[pLen], &nEnd, 2);
            pLen += 2;
            memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
            map_pData_snd(mc1, msg, pLen); // 추방된 파티원에게 전송
            map_pData_snd(c, msg, pLen); // 파티장에게 전송
            return;
        }
    } else {
        if(c->party.ison2 == 1) mc1=(type_session *)c->party.mem2;
        if(mc1!=NULL) {
            if(mc1->userNo == idx) {
                pthread_mutex_lock(&synclock);
                thr_party_set(mc1);
                c->party.cnt = 2;
                c->party.ison2 = 0;
                c->party.mem2 = NULL;
                if(c->party.ison3 == 1) mc2=(type_session *)c->party.mem3;
                if(mc2!=NULL) {
                    mc2->party.ison2 = 0;
                    mc2->party.mem2 = NULL;
                }
                pthread_mutex_unlock(&synclock);
                msg[pLen] = PK_PT_BAN;
                pLen+=1;
                msg[pLen]=1;
                pLen += 1;
                memcpy(&msg[pLen], &idx, 2);
                pLen += 2;
                memcpy(&msg[pLen], &nEnd, 2);
                pLen += 2;
                memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
                map_pData_snd(mc1, msg, pLen); // 추방된 파티원에게 전송
                map_pData_snd(mc2, msg, pLen); // 남은 파티원에게 전송
                map_pData_snd(c, msg, pLen); // 파티장에게 전송
                return;
            }
        }
        if(c->party.ison3 == 1) mc1=(type_session *)c->party.mem3;
        if(mc1!=NULL) {
            if(mc1->userNo == idx) {
                pthread_mutex_lock(&synclock);
                thr_party_set(mc1);
                c->party.cnt = 2;
                c->party.ison3 = 0;
                c->party.mem3 = NULL;
                if(c->party.ison2 == 1) mc2=(type_session *)c->party.mem2;
                if(mc2!=NULL) {
                    mc2->party.ison3 = 0;
                    mc2->party.mem3 = NULL;
                }
                pthread_mutex_unlock(&synclock);
                msg[pLen] = PK_PT_BAN;
                pLen+=1;
                msg[pLen]=1;
                pLen += 1;
                memcpy(&msg[pLen], &idx, 2);
                pLen += 2;
                memcpy(&msg[pLen], &nEnd, 2);
                pLen += 2;
                memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
                map_pData_snd(mc1, msg, pLen); // 추방된 파티원에게 전송
                map_pData_snd(mc2, msg, pLen); // 남은 파티원에게 전송
                map_pData_snd(c, msg, pLen); // 파티장에게 전송
                return;
            }
        }           
    }
    msg[pLen] = PK_PT_BAN;
    pLen+=1;
    msg[pLen]=1;
    pLen += 1;
    memcpy(&msg[pLen], &idx, 2);
    pLen += 2;
    memcpy(&msg[pLen], &nEnd, 2);
    pLen += 2;
    memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트
    map_pData_snd(c, msg, pLen); // 파티장에게 전송
}

// 파티원 탈퇴
extern void thr_party_discon(type_session *c,char flag){
    type_session *tc=NULL;
    type_session *mc1=NULL;
    type_session *mc2=NULL;

    unsigned short nEnd=PEND;
    short pLen=2;
    char msg[128];

    if(c==NULL) return;
    if(c->party.flag == 0) return;

    if(c->party.ison1 == 2) { //파티장일 경우 파티 해제
        msg[pLen] = PK_PT_BREAK;
        pLen += 1;
        memcpy(&msg[pLen], &c->userNo, 2);
        pLen += 2;
        memcpy(&msg[pLen], &nEnd, 2);
        pLen += 2;
        memcpy(&msg[0], &pLen, 2); // 패킷 길이 정보 업데이트

        if(flag==0){ //탈퇴
            pthread_mutex_lock(&synclock);
            if(c->party.ison2 == 1) {
                mc1 = (type_session *)c->party.mem2;
                if(mc1 != NULL) {
                    thr_party_set(mc1);
                }
            }
            if(c->party.ison3 == 1) {
                mc2 = (type_session *)c->party.mem3;
                if(mc2 != NULL) {
                    thr_party_set(mc2);
                }
            }
            thr_party_set(c);
            pthread_mutex_unlock(&synclock);

            map_pData_snd(c, msg, pLen);
        } else {
            if(c->party.ison2 == 1) {
                mc1 = (type_session *)c->party.mem2;
                if(mc1 != NULL) {
                    thr_party_set(mc1);
                }
            }
            if(c->party.ison3 == 1) {
                mc2 = (type_session *)c->party.mem3;
                if(mc2 != NULL) {
                    thr_party_set(mc2);
                }
            }            
        }
        if(mc1) map_pData_snd(mc1, msg, pLen); //파티 깨짐을 알림
        if(mc2) map_pData_snd(mc2, msg, pLen);

    } else {
        tc=(type_session *)c->party.master;
        if(tc != NULL) {
            if(tc->party.cnt == 2) { //파티 해체
                msg[pLen] = PK_PT_BREAK;
                pLen += 1;
                memcpy(&msg[pLen], &c->userNo, 2);
                pLen += 2;
                memcpy(&msg[pLen], &nEnd, 2);
                pLen += 2;
                memcpy(&msg[0], &pLen, 2); 

                if(flag == 0) {
                    pthread_mutex_lock(&synclock);
                    if(tc->party.ison2 == 1) {
                        mc1 = (type_session *)tc->party.mem2;
                        if(mc1 != NULL) {
                            thr_party_set(mc1);
                        }
                    }
                    if(tc->party.ison3 == 1) {
                        mc2 = (type_session *)tc->party.mem3;
                        if(mc2 != NULL) {
                            thr_party_set(mc2);
                        }
                    }
                    thr_party_set(tc);
                    pthread_mutex_unlock(&synclock);                    
                } else {
                    if(tc->party.ison2 == 1) {
                        mc1 = (type_session *)tc->party.mem2;
                        if(mc1 != NULL) {
                            thr_party_set(mc1);
                        }
                    }
                    if(tc->party.ison3 == 1) {
                        mc2 = (type_session *)tc->party.mem3;
                        if(mc2 != NULL) {
                            thr_party_set(mc2);
                        }
                    }
                    thr_party_set(tc);
                }
                map_pData_snd(tc, msg, pLen); 
                if(mc1) map_pData_snd(mc1, msg, pLen); 
                if(mc2) map_pData_snd(mc2, msg, pLen);
            } else {
                msg[pLen] = PK_PT_LEAVE;
                pLen += 1;
                msg[pLen] = 0;
                pLen += 1;
                memcpy(&msg[pLen], &c->userNo, 2);
                pLen += 2;
                memcpy(&msg[pLen], &nEnd, 2);
                pLen += 2;
                memcpy(&msg[0], &pLen, 2); 

                tc=(type_session *)c->party.master;
                if(tc==NULL) return;

                if(flag==0) {
                    pthread_mutex_lock(&synclock);
                    if(tc->party.ison2 == 1) {
                        mc1 = (type_session *)tc->party.mem2;
                        if(mc1 == c) {
                            thr_party_set(mc1);
                            tc->party.ison1 = 0;
                            tc->party.cnt=2;
                            tc->party.mem2 = NULL;
                            tc->party.ison2 = 0;
                        }else {
                            thr_party_set(c);
                            pthread_mutex_unlock(&synclock);
                            return;                          
                        }                        
                    } else {
                        mc2 = (type_session *)tc->party.mem3;
                        if(mc2 == c) {
                            thr_party_set(mc2);
                            tc->party.ison1 = 0;
                            tc->party.cnt=2;
                            tc->party.mem2 = NULL;
                            tc->party.ison2 = 0;
                        } else {
                            thr_party_set(c);
                            pthread_mutex_unlock(&synclock);
                            return;
                        }
                    }
                    pthread_mutex_unlock(&synclock);
                    if(mc1) map_pData_snd(mc1, msg, pLen); 
                    if(mc2) map_pData_snd(mc2, msg, pLen); 
                    map_pData_snd(tc, msg, pLen); 
                } else {
                    if(tc->party.ison2 == 1) {
                        mc1 = (type_session *)tc->party.mem2;
                        if(mc1 == c) {
                            thr_party_set(mc1);
                            tc->party.ison1 = 0;
                            tc->party.cnt=2;
                            tc->party.mem2 = NULL;
                            tc->party.ison2 = 0;
                        } else {
                            thr_party_set(c);
                            pthread_mutex_unlock(&synclock);
                            return;
                        }
                    } else {
                        mc2 = (type_session *)tc->party.mem3;
                        if(mc2 == c) {
                            thr_party_set(mc2);
                            tc->party.ison1 = 0;
                            tc->party.cnt=2;
                            tc->party.mem2 = NULL;
                            tc->party.ison2 = 0;
                        } else {
                            thr_party_set(c);
                            pthread_mutex_unlock(&synclock);
                            return;
                        }
                    }
                }
                if(mc1&&mc1!=c) map_pData_snd(mc1, msg, pLen);
                if(mc2&&mc2!=c) map_pData_snd(mc2, msg, pLen);
                map_pData_snd(tc, msg, pLen); 

            }
        }
    }
}

static void thr_map_move_req(type_session *c, char *dat, int udpsock){
    unsigned char gidx=0, res=0;
    int dist=0, udlen=0;
    unsigned int sKey=0, svrIdx;
    type_sockadr *udsock;
    short Len=2;
    char msg[64];
    unsigned short nEnd=PEND;

    gidx=dat[3];

    if(gidx>7||gidx==_MAP_IDX) res = 1;
    else {
        switch(gidx) {
            case 0:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_0_X, _MAP_GWAY_0_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_0_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_0_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_0_TO_SVR);
                }
                break;
            case 1:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_1_X, _MAP_GWAY_1_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_1_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_1_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_1_TO_SVR);
                }
                break;
            case 2:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_2_X, _MAP_GWAY_2_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_2_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_2_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_2_TO_SVR);
                }
                break;
            case 3:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_3_X, _MAP_GWAY_3_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_3_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_3_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_3_TO_SVR);
                }
                break;
            case 4:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_4_X, _MAP_GWAY_4_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_4_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_4_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_4_TO_SVR);
                }
                break;
            case 5:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_5_X, _MAP_GWAY_5_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_5_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_5_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_5_TO_SVR);
                }
                break;
            case 6:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_6_X, _MAP_GWAY_6_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_6_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_6_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_6_TO_SVR);
                }
                break;
            case 7:
                dist=map_ret_dist(c->Cx, c->Cy, _MAP_GWAY_7_X, _MAP_GWAY_7_Y);
                if(dist>10) res = 2;
                else {
                    svrIdx = _MAP_GWAY_7_TO_SVR;
                    udsock=db_ret_udpsock(_MAP_GWAY_7_TO_SVR);
                    udlen=db_ret_udpLen(_MAP_GWAY_7_TO_SVR);
                }
                break;
        }
    }

    if(res==0) {
        sKey = ((unsigned int)rand())^((unsigned int)time(NULL));
        msg[Len]=PKU_MAP_KEY;
        Len+=1;
        msg[Len]=_MAP_IDX;
        Len+=1;
        memcpy(&msg[Len], &c->userNo,2);
        Len+=2;
        memcpy(&msg[Len], &sKey,4);
        Len+=4;
        msg[Len]=c->uLen;
        Len+=1;
        memcpy(&msg[Len], &c->userid[0], c->uLen);
        Len+=c->uLen;
        memcpy(&msg[Len], &nEnd, 2);
        Len+=2;
        memcpy(&msg[0], &Len, 2);
        sendto(udpsock, msg, Len, 0, (SA*)udsock, udlen);
    } else {
        msg[Len]=PK_MAP_MOVE_REQ;
        Len+=1;
        msg[Len]=res;
        Len+=1;
        memcpy(&msg[Len], &nEnd, 2);
        Len+=2;
        memcpy(&msg[0], &Len, 2);
        map_pData_snd(c, msg, Len);
    }
}