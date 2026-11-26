#include <stdio.h>
#include <string.h>

#include "npc.h"
#include "map.h"
#include "publicInc.h"

static type_npc 	 * npcArray[_MAX_MONSTER_NO];
static type_npcmap	mapNpcArray[S_WIDTH][S_WIDTH];

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

extern void npc_mv_set(unsigned short idx) //일반 이동
{
    int res=0;

    if(npcArray[idx]->move_F_cnt>=NPC_MV_FRAME) {
        //첫 프레임에서 경로 찾기나 움직일 수 있는지를 구한다. 충돌 체크
        npcArray[idx]->move_F_cnt = 0;
        res = npc_mv_one(npcArray[idx]); //카운트가 다 되었으니 한 칸 이동

        if(npcArray[idx]->mvstatus == 0) { //처음 접할 때
            if(res==1) {
                if(npcArray[idx]->x == npcArray[idx]->dx && npcArray[idx]->y == npcArray[idx]->dy) { //목적지 도착
                    if(npcArray[idx]->tgFlag == 0) npcArray[idx]->status = S_STAND;
                    else npcArray[idx]->status = S_FIGHT;
                    npcArray[idx]->mvstatus = 0; //이동
                } else npcArray[idx]->mvstatus = 1; //이동
            }
        } else {
            if(res==0) { //섹터 이동 실패
                npcArray[idx]->status = S_STAND;
                npcArray[idx]->mvstatus = 0; //이동                
            } else { //이동 성공시
                if(npcArray[idx]->x == npcArray[idx]->dx && npcArray[idx]->y == npcArray[idx]->dy) { //목적지 도착
                    if(npcArray[idx]->tgFlag == 0) npcArray[idx]->status = S_STAND;
                    else npcArray[idx]->status = S_FIGHT;
                    npcArray[idx]->mvstatus = 0; //이동
                }
            }
        }
    } else npcArray[idx]->move_F_cnt++;
}

extern int npc_mv_one(type_npc * npc)   //한칸만 이동
{
    type_objlist **pos;
    type_objlist *tmp;
    short tmpx, tmpy;
    unsigned short nEnd = PEND;
    char mesg[64];
    short rLen=2;
    short DIRx, DIRy;

    if(npc->x == npc->dx && npc->y == npc->dy) { //목표지점 도착
        return 1;
    }

    DIRx = npc->x - npc->dx;
    DIRy = npc->y - npc->dy;
    tmpx = npc->x;
    tmpy = npc->y;

    if(DIRx==0 && DIRy < 0) {
        npc->DirLoc = 4; //12시 방향
        tmpy +=1;
    } else if(DIRx<0 && DIRy<0) {
        npc->DirLoc=3; //1시 방향
        tmpx +=1;
        tmpy +=1;
    } else if(DIRx<0 && DIRy==0) {
        npc->DirLoc=2; //3시 방향
        tmpx +=1;
    } else if(DIRx<0 && DIRy>0) {
        npc->DirLoc=1; //5시 방향
        tmpx +=1;
        tmpy -=1;
    } else if(DIRx==0 && DIRy>0) {
        npc->DirLoc=0; //6시 방향
        tmpy -=1;
    } else if(DIRx>0 && DIRy>0) {
        npc->DirLoc=7; //7시 방향
        tmpx -=1;
        tmpy -=1;
    } else if(DIRx>0 && DIRy==0) {
        npc->DirLoc=6; //9시 방향
        tmpx -=1;
    } else if(DIRx>0 && DIRy<0) {
        npc->DirLoc=5; //11시 방향
        tmpx -=1;
        tmpy +=1;
    }

    //이 부분에 길찾기를 넣을 수 있다.
    if(npc->npc_id==17) printf("%d npc %d, %d -> %d,%d로 한 칸이동\n", npc->npc_id, npc->x, npc->y, tmpx, tmpy);
    if(tmpx<0) tmpx = 0;
    if(tmpy<0) tmpy = 0;
    npc->x = tmpx;
    npc->y = tmpy;
    if(npc->x >= M_SIZE_X|| npc->y >= M_SIZE_X) {
        npc->status = S_STAND;
    }

    if(npc->mvstatus == 0) { //0이면 처음 움직이는 것이므로 패킷 발송해야 함
        mesg[rLen] = PK_OBJ_UPDATE_ACT;
        rLen+= 1;

        mesg[rLen] = T_NPC;
        rLen+=1;

        mesg[rLen] = 1; //카운트
        rLen+=1;

        mesg[rLen] = npc->div;
        rLen+=1;

        memcpy(&mesg[rLen], &npc->npc_id,2);
        rLen+=2;

        mesg[rLen] = S_MOVE;
        rLen+=1;

        mesg[rLen] = npc->DirLoc;
        rLen+=1;

        mesg[rLen] = npc->speed;
        rLen+=1;

        memcpy(&mesg[rLen], &npc->x, 2);
        rLen+=2;
        memcpy(&mesg[rLen], &npc->y, 2);
        rLen+=2;

        memcpy(&mesg[rLen], &npc->dx, 2);
        rLen+=2;
        memcpy(&mesg[rLen], &npc->dy, 2);
        rLen+=2;

        memcpy(&mesg[rLen], &nEnd, 2);
        rLen+=2;
        memcpy(&mesg[0], &rLen, 2);
        map_usersend_All(npc->ARx, npc->ARy, mesg, rLen, NULL);
        npc->mvstatus = 1;
    }

    tmpx = npc->x/S_UNITSIZE;
    tmpy = npc->y/S_UNITSIZE;

    if(tmpx!=npc->ARx || tmpy!=npc->ARy) {
        if( !npc) {
            printf("npc pointer null\n");
            return 0;
        }

        if(mapNpcArray[npc->ARx][npc->ARy].objL == NULL) {
            printf("mapArray NULL list in map_char_mv\n");
            return 0;
        }

        pos = find_node_objlist(&(mapNpcArray[npc->ARx][npc->ARy].objL), npc);
        if(pos == NULL){
            printf("pos not found\n");
            return 0;
        }

        tmp = pop_data(pos);
        if(!tmp) {
            printf("no pop\n");
            return 0;
        }
        insert_node_objlist(&(mapNpcArray[tmpx][tmpy].objL), tmp);

        npc->ARx = tmpx;
        npc->ARy = tmpy;

        npc_mv_addinfo(npc);
    }
    return 1;
}

extern void npc_mv_addinfo(type_npc * npc)
{
    
}