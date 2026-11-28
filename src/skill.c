#include <stdlib.h>
#include <string.h>

#include "session.h"
#include "map.h"
#include "skill.h"
#include "npc.h"
#include "publicInc.h"

struct skills{
    unsigned char type; // 타입 0: 물리 계열 공격 1: 중독 계열 공격 2: 상태 변화 계열
    unsigned char skillidx; //스킬 번호

    unsigned char lvpwr[11]; //레벨에 따른 공격력

    unsigned int lvupcost[11]; // 스킬 레벨업에 드는 돈
    unsigned int getcoin; //스킬을 배울 때 드는 돈
} skill_def[4];

extern void skill_defset(int type, int skillidx, char *lvpwr, char *lvupcost, int coin)
{
    int i;

    if(skillidx>3) return;
    skill_def[skillidx].type = type;
    skill_def[skillidx].skillidx = skillidx;
    skill_def[skillidx].getcoin = coin;

    for(i=1;i<11;i++)
    {
        if(i==1) skill_def[skillidx].lvpwr[i] = atoi(strtok(lvpwr, "|"));
        else skill_def[skillidx].lvpwr[i] = atoi(strtok(NULL, "|"));
    }
    for(i=1;i<11;i++)
    {
        if(i==1) skill_def[skillidx].lvupcost[i] = atoi(strtok(lvupcost, "|"));
        else skill_def[skillidx].lvupcost[i] = atoi(strtok(NULL, "|"));
    }
}

//스킬 배우기 패킷 처리
extern void skill_get(void *chr, char *dat){
    unsigned char skidx;
    int i,res=0,invidx=0,chk=0;
    type_session *c;
    unsigned short nEnd = PEND;
    char msg[32];
    unsigned short Len=2;

    c=(type_session *)chr;
    skidx = dat[3];
    if(skidx>3) res=1;
    else if(skill_def[skidx].getcoin > c->coin) res=2; // 돈 부족
    for(i=0;i<4;i++) {
        if(c->skill[i] == 0 && chk == 0){
            invidx=i;
            chk=1;
        }
        if(c->skill[i] == skidx){ 
            res=3; 
            break;
        } // 이미 배운 스킬
    }

    if(res==0&&chk==1){
        pthread_mutex_lock(&synclock);
        c->skill[invidx] = skidx;
        c->skill_lv[invidx] = 1; // 스킬 레벨 1로 설정
        c->coin -= skill_def[skidx].getcoin; // 스킬 구매 비용 차감        
        pthread_mutex_unlock(&synclock);
    } else res=4;

    msg[Len] = PK_SKILL_GET;
    Len+=1;
    msg[Len] = res;
    Len+=1;
    msg[Len]=invidx;
    Len+=1;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

//스킬 레벨업
extern void skill_lvup(void *chr, char *dat){
    unsigned char invidx, skidx=0, lv;
    int res=0;
    type_session *c;
    unsigned short nEnd = PEND;
    char msg[32];
    unsigned short Len=2;

    c=(type_session *)chr;
    invidx = dat[3];
    if((skidx=c->skill[invidx]) == 0) res=2; // 스킬이 없음
    else if((lv=c->skill_lv[invidx]) >= 10) res=3; // 최대 레벨 도달
    else if(skill_def[skidx].lvupcost[lv] > c->coin) res=4; // 돈 부족
    else {
        pthread_mutex_lock(&synclock);
        c->coin -= skill_def[skidx].lvupcost[lv]; // 스킬 레벨업 비용 차감
        c->skill_lv[invidx] += 1; // 스킬 레벨 증가
        pthread_mutex_unlock(&synclock);
    }

    msg[Len] = PK_SKILL_LVUP;
    Len+=1;
    msg[Len] = res;
    Len+=1;
    msg[Len] = skidx;
    Len+=1;
    msg[Len] = invidx;
    Len+=1;
    msg[Len]=c->skill_lv[invidx];
    Len+=1;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

//스킬 사용
extern void skill_use(void *chr, char *dat, void *conn){
    unsigned short x,y,Len=2;
    unsigned char invidx, res=0;
    type_session *c;
    char NPCDat[512], USRDat[512], msg[1024];
    unsigned char NPCcnt=0, USRcnt=0, tot=0;
    short lv;
    unsigned short nEnd = PEND;

    c=(type_session *)chr;

    invidx = dat[3];
    if(invidx>3) return;
    if(c->mana_c<0) return;
    else {
        pthread_mutex_lock(&synclock);
        c->mana_c-=10;
        pthread_mutex_unlock(&synclock);
    }
    memcpy(&x, &dat[4], 2);
    memcpy(&y, &dat[6], 2);

    msg[Len] = PK_SKILL_USE;
    Len+=1;
    msg[Len] = res;
    Len+=1;
    msg[Len] = T_NPC;
    Len+=1;
    memcpy(&msg[Len],&c->userNo,2);
    Len+=2;

    switch(c->skill[invidx]) {
        case 1: //1번 스킬
            msg[Len]=1;
            Len+=1;
            memcpy(&msg[Len], &x, 2);
            Len+=2;
            memcpy(&msg[Len], &y, 2);
            Len+=2;
            Len+=1; // 객체 수
            if(c->pkmode==1){

            }else{
                lv=c->skill_lv[invidx];
                npc_skill_1(c, skill_def[1].lvpwr[lv], x, y, NPCDat, conn);
                NPCcnt=NPCDat[0];
            }
            if(NPCcnt>0){
                tot+=NPCcnt;
                memcpy(&msg[Len], &NPCDat[1], NPCcnt*5);
                Len += NPCcnt*5;
            }
            if(USRcnt>0){
                tot+=USRcnt;
                memcpy(&msg[Len], &USRDat[1], USRcnt*5);
                Len += USRcnt*5;
            }
            break;
        case 2:
            if(c->pkmode == 1){

            }else {

            }
            break;
        case 3:
            if(c->pkmode == 1){

            }else {

            }
            break;
    }
    msg[12]=tot;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_usersend_All(c->Ax, c->Ay, msg, Len, NULL);
}