#include <stdio.h>

#include "npc.h"
#include "map.h"
#include "publicInc.h"

static type_npc 	 * npcArray[_MAX_MONSTER_NO];

static int				npc_cnt=0;

static type_gnpc def_mon[21]; //몬스터 정보 리스트
static type_gnpc def_npc[1]; //NPC 정보 리스트

extern void npc_set_def(char div, short no, char gtype, char level, char speed)
{
    if(div==0) { // NPC일 경우
        def_npc[no].no = no;
        def_npc[no].gtype = gtype;
        def_npc[no].level = level;
        def_npc[no].speed = speed;
    } else { // 몬스터일 경우
        def_mon[no].no = no;
        def_mon[no].gtype = gtype;
        def_mon[no].level = level;
        def_mon[no].speed = speed;
    }
}

extern void npc_set(char div, short no, char x, char y)
{
    
}

extern void npc_3x3_sectorinfo(type_session *c) {
}

extern void npc_chr_mv_addinfo(type_session *c) {
}

void * Thread_npc_sync(void *arg){
    int i,j;
    short rotate_time = GAME_FRAME*5;
    short Rest = 0,torf=0;
    for(i=0;i<npc_cnt; i++)
    {
        if(npcArray[i]->status == S_MOVE) { //움직이는 상태
            if(npcArray[i]->dx!=npcArray[i]->x||npcArray[i]->dy!=npcArray[i]->y) {
                npc_mv_set(i); //이동처리
                continue;
            } else { //목적지에 도착한 NPC
                npcArray[i]->move_F_cnt = 0;
                npcArray[i]->rotate_F_cnt = 0;
                npcArray[i]->status = S_STAND;
                npcArray[i]->mvstatus = 0;
            }   
        } else {
            if(npcArray[i]->rotate_F_cnt > rotate_time){ //움직일 타이밍일때
                npcArray[i]->ret_F_cnt+=1;
                if(npcArray[i]->ret_F_cnt>10){ //10번 돌아다녔으면 생성된 위치로 돌아감
                    if(i==17) printf("NPC 8 생성지점으로 되돌아가는 타이밍\n");
                    npcArray[i]->ret_F_cnt = 0;
                    npcArray[i]->dx = npcArray[i]->Gx;
                    npcArray[i]->dy = npcArray[i]->Gy;
                } else { //순회할 목적지를 찾는다.
                    Rest+=1;
                    if(Rest>7) Rest=0;
                    switch(Rest) {
                        case 0:
                            npcArray[i]->dx = npcArray[i]->x-5;
                            npcArray[i]->dy = npcArray[i]->y;
                            break;
                        case 1:
                            npcArray[i]->dx = npcArray[i]->x-5;
                            npcArray[i]->dy = npcArray[i]->y+5;
                            break;
                        case 2:
                            npcArray[i]->dx = npcArray[i]->x+5;
                            npcArray[i]->dy = npcArray[i]->y-5;
                            break;
                        case 3:
                            npcArray[i]->dx = npcArray[i]->x-5;
                            npcArray[i]->dy = npcArray[i]->y-5;
                            break;
                        case 4:
                            npcArray[i]->dx = npcArray[i]->x+5;
                            npcArray[i]->dy = npcArray[i]->y+5;
                            break;
                        case 5:
                            npcArray[i]->dx = npcArray[i]->x;
                            npcArray[i]->dy = npcArray[i]->y-5;
                            break;
                        case 6:
                            npcArray[i]->dx = npcArray[i]->x+5;
                            npcArray[i]->dy = npcArray[i]->y;
                            break;
                        case 7:
                            npcArray[i]->dx = npcArray[i]->x;
                            npcArray[i]->dy = npcArray[i]->y+5;
                            break;
                    }
                }
                j=0;
                while(1) {
                    torf = map_isgo(npcArray[i]->dx, npcArray[i]->dy); //이동 가능 지역 찾기
                    if(torf==0) { //이동 가능
                        if(i==17) printf("%d번 NPC가 이동 세팅이 됨 현재: %d, %d 목표지점: %d, %d\n", i, npcArray[i]->x, npcArray[i]->y, npcArray[i]->dx, npcArray[i]->dy);
                        npcArray[i]->status = S_MOVE;
                        npcArray[i]->mvstatus = 0;
                        break; // 이동 가능하면 while문 탈출
                    } else { //이동 불가
                        if(npcArray[i]->dx>=M_SIZE_X) npcArray[i]->dx = npcArray[i]->dx+5;
                        else if(npcArray[i]->dy>=M_SIZE_X) npcArray[i]->dy = npcArray[i]->dy+5;
                        else {
                            npcArray[i]->dx = npcArray[i]->dx+5;
                            npcArray[i]->dy = npcArray[i]->dy+5;
                        }
                        if(i==17) printf("%d번 NPC가 이동 불가라서 목표지점 다시 찾음 현재: %d, %d 목표지점 %d, %d\n", i, npcArray[i]->x, npcArray[i]->y, npcArray[i]->dx, npcArray[i]->dy);                        
                    }
                    if(j==20) { //무한루프 방지
                        npcArray[i]->status = S_STAND;
                        break;
                    }
                    j++;
                }

                if(npcArray[i]->dx==npcArray[i]->x && npcArray[i]->dy==npcArray[i]->y) {
                    npcArray[i]->status = S_STAND;
                } else {
                    npcArray[i]->mvstatus = 0;
                    npcArray[i]->status = S_MOVE;
                }
                npcArray[i]->rotate_F_cnt = 0;               
            } else npcArray[i]->rotate_F_cnt++;
        }
    }
}

extern void npc_mv_set(unsigned short idx)
{

}