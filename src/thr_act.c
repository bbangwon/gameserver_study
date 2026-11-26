#include <pthread.h>
#include <stdio.h>
#include <string.h>
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