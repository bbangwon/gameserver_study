#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "db.h"
#include "npc.h"
#include "map.h"
#include "item.h"
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
    type_session * tgChr;
    type_npc * tmpNpc;
    short rotate_time = GAME_FRAME*5;
    short Rest = 0,torf=0,dist=0;
    for(i=0;i<npc_cnt; i++)
    {
        if(npcArray[i]->status == S_FIGHT) {
            tgChr = (type_session *)npcArray[i]->targetChr;
            dist=map_ret_dist(npcArray[i]->x, npcArray[i]->y, tgChr->Cx, tgChr->Cy); //npc와 캐릭터간 거리 반환

            if(dist < 0) {
                npcArray[i]->targetChr = NULL;
                npcArray[i]->status = S_STAND;
                npcArray[i]->mvstatus = 0;
                continue;
            }

            if(npcArray[i]->tgLen>=dist) {
                //타격 거리 내에 있으므로 타격한다.
                if(npcArray[i]->heat_F_cnt>=_ONE_SEC){
                    npc_attack_chr(tgChr, i); //공격
                    npcArray[i]->move_F_cnt = 0;
                    npcArray[i]->heat_F_cnt = 0;
                    npcArray[i]->wait_F_cnt = 0;
                    npcArray[i]->mvstatus = 0;
                } else npcArray[i]->heat_F_cnt++;                
            } else if(dist<_NPC_TGT_SIG) { // 추격 범위 내에 있을 경우
                npc_mv_set(i);
                continue;
            } else { //타겟이 시야에서 사라졌을 때
                npcArray[i]->status = S_STAND;
                npcArray[i]->mvstatus = 0;
                npcArray[i]->tgFlag = 0;
            }
            continue;
        } else {
            if(npcArray[i]->bat_type == T_ACTIVE) {
                //선공하는 몬스터의 처리(해당 사항 있으면 행동하고, 아니면 인공지능)
                if((npcArray[i]->targetChr = map_ret_Activ_tg(npcArray[i]->x, npcArray[i]->y, npcArray[i]->Sight)) != NULL){
                    //근처에 타겟이 있있을때 전투 모드로 들어간다.
                    tgChr = (type_session *)npcArray[i]->targetChr;
                    npcArray[i]->targetNO = tgChr->userNo;
                    npcArray[i]->status = S_FIGHT;
                    npcArray[i]->tgFlag = 1;
                    continue;
                }
            } else if(npcArray[i]->bat_type == T_GROUP){
                //집단형 npc(선공은 아니지만 주위에서 싸우면 같이 가서 싸움)
                if((tmpNpc=npc_ret_fight(i)) != NULL) {
                    npcArray[i]->targetNO = tmpNpc->targetNO;
                    npcArray[i]->targetChr = tmpNpc->targetChr;
                    npcArray[i]->status = S_FIGHT;
                    npcArray[i]->tgFlag=1;
                }
            }

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

extern int npc_ret_dist(unsigned short x, unsigned short y, unsigned short idx)
{
    return 0;
}

extern short npc_chr_attack_npc(void * chr, unsigned short Index,void * conn)
{
    int rLen = 2;
    short result = 0, wasLVup = 0;
    short DaMaGe;
    int exp=0,coin=0;
    type_session *c, *mc, *pc;
    char mesg[32];
    unsigned short nEnd = PEND;
    int pt0, pt1, pt2;

    c = (type_session *)chr;
    if(Index>_MAX_MONSTER_NO) return 0;
    if(npcArray[Index]->div == 1) return 1;
    if(npcArray[Index]->status == S_DIE) return 0; //이미 죽은 몬스터

    pthread_mutex_lock(&npclock);
    npcArray[Index]->targetNO = c->userNo;
    npcArray[Index]->targetChr = c;
    npcArray[Index]->mvstatus = 0;
    npcArray[Index]->status = S_FIGHT;

    DaMaGe = c->attack - npcArray[Index]->DP;    //데미지 기본 공식 기본 물리 공격
    if(DaMaGe<0) DaMaGe = 1;

    npcArray[Index]->HP=npcArray[Index]->HP-DaMaGe;

    if(npcArray[Index]->HP<0) {
        result = 1; // 몬스터 사망
        npcArray[Index]->HP=0;
        npcArray[Index]->targetNO = 0;
        npcArray[Index]->targetChr = NULL;
        npcArray[Index]->status = S_DIE;
        pthread_mutex_unlock(&npclock);

        exp = npcArray[Index]->EXP;
        coin += npcArray[Index]->level*10;
    }else {
        pthread_mutex_unlock(&npclock);

        pthread_mutex_lock(&synclock);
        c->userStat = S_FIGHT;
        c->tgtype = 1; //npc/usr
        c->target = npcArray[Index];
        c->target_id = Index;
        c->attack_F_cnt=0;
        pthread_mutex_unlock(&synclock);
    }
    mesg[rLen] = PK_OBJ_UPDATE_ACT;
    rLen+=1;
    mesg[rLen] = T_USER;
    rLen+=1;
    mesg[rLen] = 1;
    rLen+=1;
    memcpy(&mesg[rLen], &c->userNo, sizeof(c->userNo));
    rLen+=2;
    mesg[rLen]=S_FIGHT;
    rLen+=1;
    mesg[rLen]=c->Dir;
    rLen+=1;
    mesg[rLen]=T_NPC;
    rLen+=1;
    memcpy(&mesg[rLen], &Index, 2);
    rLen+=2;
    memcpy(&mesg[rLen], &npcArray[Index]->HP, 2);
    rLen+=2;
    if(npcArray[Index]->HP == 0) {
        memcpy(&mesg[rLen], &c->exp, 4);
        rLen+=4;
    }
    memcpy(&mesg[rLen], &nEnd, 2);
    rLen+=2;
    memcpy(&mesg[0], &rLen, 2);
    map_usersend_All(c->Ax, c->Ay, mesg, rLen, NULL);

    if(result==1) { //npc는 죽으면서 아이템을 만든다.
        npc_drop_item(npcArray[Index], c->userNo);

        if(c->party.flag==1) { //파티중이면 경험치 공유
            if(c->party.ison1==2) //자신이 파티장일때
            {
                coin = coin / c->party.cnt;
                exp = exp / c->party.cnt;
                mc=c;
            } else {
                mc=(type_session *)c->party.master;
                if(mc==NULL) return 0;
                coin = coin / c->party.cnt;
                exp = exp / c->party.cnt;
            }
            pthread_mutex_lock(&synclock);
            mc->coin += coin;
            mc->exp += exp;
            pt0=db_chk_lvup(mc,conn);

            if(mc->party.ison2==1){
                pc=(type_session *)mc->party.mem2;
                if(pc==NULL) {
                    pthread_mutex_unlock(&synclock);
                    return 0;
                }
                pc->coin+=coin;
                pc->exp+=exp;
                pt1=db_chk_lvup(pc,conn);
            }
            if(mc->party.ison3==1) {
                pc=(type_session *)mc->party.mem3;
                if(pc==NULL){
                    pthread_mutex_unlock(&synclock);
                    return 0;
                }
                pc->coin+=coin;
                pc->exp+=exp;
                pt2=db_chk_lvup(pc,conn);
            }
            pthread_mutex_unlock(&synclock);

            if(pt0==1) {
                rLen = 2;
                mesg[rLen] = PK_LEVELUP;
                rLen+=1;
                mesg[rLen] = mc->level;
                rLen+=1;
                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;
                memcpy(&mesg[0], &rLen, 2);
                map_pData_snd(mc, mesg, rLen);
            }
            if(pt1==1) {
                pc=(type_session *)mc->party.mem2;
                if(pc==NULL) return 0;
                rLen = 2;
                mesg[rLen] = PK_LEVELUP;
                rLen+=1;
                mesg[rLen] = pc->level;
                rLen+=1;
                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;
                memcpy(&mesg[0], &rLen, 2);
                map_pData_snd(pc, mesg, rLen);
            }
            if(pt2==1) {
                pc=(type_session *)mc->party.mem3;
                if(pc==NULL) return 0;
                rLen = 2;
                mesg[rLen] = PK_LEVELUP;
                rLen+=1;
                mesg[rLen] = pc->level;
                rLen+=1;
                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;
                memcpy(&mesg[0], &rLen, 2);
                map_pData_snd(pc, mesg, rLen);
            }
        } else {
            pthread_mutex_lock(&synclock);
            c->coin += coin;
            c->exp += exp;
            wasLVup = db_chk_lvup(c, conn);
            pthread_mutex_unlock(&synclock);

            if(wasLVup==1) {
                rLen = 2;
                mesg[rLen] = PK_LEVELUP;
                rLen+=1;
                mesg[rLen] = c->level;
                rLen+=1;
                memcpy(&mesg[rLen], &nEnd, 2);
                rLen+=2;
                memcpy(&mesg[0], &rLen, 2);
                map_pData_snd(c, mesg, rLen);
            }           
        }
    }
    return 1;   
}

extern short npc_drop_item(type_npc * npc, unsigned short userNo)
{
    type_itemLst_field *temp;   
    unsigned int ItmNo;
    int tmp=0, cnt=0, loopCnt=0, i;
    short dLen=2;
    char data[128];
    unsigned short nEnd = PEND;

    tmp = (((unsigned int)rand())^((unsigned int)time(NULL)))%100;
    // 10% 확률로 아이템 2개 획득
    if(tmp < 10) loopCnt=2;
    else loopCnt=1;

    data[dLen] = PK_OBJ_ADD;
    dLen+=1;
    data[dLen] = T_ITEM;
    dLen+=1;
    // data[dLen] = 1;
    dLen+=1;

    for(i=0;i<12;i++) {
        tmp = (((unsigned int)rand())^((unsigned int)time(NULL)))%100;
        if(npc->div==1) { //NPC
            if(def_npc[npc->npc_id].rate[i] >= tmp){
                if(def_npc[npc->npc_id].item[i]==0) continue; //생성 아이템이 없음
                cnt+=1;
                if(!(temp = malloc(sizeof(type_itemLst_field)))) return 0;

                temp->rmFcnt = 0;
                temp->rmHcnt = 0;

                pthread_mutex_lock(&itemlock);
                temp->item_idx_cnt=item_ret_default_cnt(def_npc[npc->npc_id].item[i]);
                if(temp->item_idx_cnt==0){
                    free(temp);
                    pthread_mutex_unlock(&itemlock);
                    continue;
                }
                ItmNo = retNullitemIdx();
                setitemIdx(ItmNo);
                temp->idx = ItmNo;
                item_add_sect(npc->ARx, npc->ARy, temp);
                temp->x=npc->x;
                temp->y=npc->y;
                temp->item_idx=def_npc[npc->npc_id].item[i];
                pthread_mutex_unlock(&itemlock);

                memcpy(&data[dLen], &temp->idx, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->x, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->y, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->item_idx, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->item_idx_cnt, 2);
                dLen+=2;
            }
        } else {
            //몬스터
            if(def_mon[npc->npc_id].rate[i] >= tmp) {
                if(def_mon[npc->npc_id].item[i]==0) continue;
                cnt+=1;

                if(!(temp = malloc(sizeof(type_itemLst_field)))) return 0;

                temp->rmFcnt = 0;
                temp->rmHcnt = 0;

                pthread_mutex_lock(&itemlock);
                temp->item_idx_cnt=item_ret_default_cnt(def_mon[npc->npc_id].item[i]);
                if(temp->item_idx_cnt==0){
                    free(temp);
                    pthread_mutex_unlock(&itemlock);
                    continue;
                }
                ItmNo = retNullitemIdx();
                setitemIdx(ItmNo);
                temp->idx = ItmNo;
                item_add_sect(npc->ARx, npc->ARy, temp);
                temp->x=npc->x;
                temp->y=npc->y;
                temp->item_idx=def_mon[npc->npc_id].item[i];
                pthread_mutex_unlock(&itemlock);

                memcpy(&data[dLen], &temp->idx, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->x, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->y, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->item_idx, 2);
                dLen+=2;
                memcpy(&data[dLen], &temp->item_idx_cnt, 2);
                dLen+=2;
            }
        }
        if(cnt==loopCnt) break;
    }
    data[4] = cnt;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_usersend_All(npc->ARx, npc->ARy, data, dLen, NULL);
    return 1;
}

extern void npc_attack_chr(void * chrr, unsigned short idx)
{
    short DaMaGe, DIRx, DIRy, HP;
    char mesg[64];
    unsigned short rLen = 2;
    unsigned short nEnd = PEND;
    type_session *chr;

    if(idx>=_MAX_MONSTER_NO) return;
    if(chrr==NULL){
        npcArray[idx]->status=S_STAND;
        npcArray[idx]->dx=npcArray[idx]->Gx;
        npcArray[idx]->dy=npcArray[idx]->Gy;
        npcArray[idx]->targetChr = NULL;
        npcArray[idx]->tgFlag = 0;
        return;
    }

    if(npcArray[idx]->status==S_DIE) return;
    chr = (type_session *)chrr;
    if(chr->userStat == S_DIE){
        npcArray[idx]->status = S_STAND;
        npcArray[idx]->dx = npcArray[idx]->Gx;
        npcArray[idx]->dy = npcArray[idx]->Gy;
        npcArray[idx]->targetChr = NULL;
        npcArray[idx]->tgFlag = 0;
        return;
    }
    DIRx = npcArray[idx]->x - chr->Cx;
    DIRy = npcArray[idx]->y - chr->Cy;

    if(DIRx==0 && DIRy<0) npcArray[idx]->DirLoc=4; //12시방향
    else if(DIRx<0 && DIRy<0) npcArray[idx]->DirLoc=3; //1시방향
    else if(DIRx<0 && DIRy==0) npcArray[idx]->DirLoc=2; //3시방향
    else if(DIRx<0 && DIRy>0) npcArray[idx]->DirLoc=1; //5시방향
    else if(DIRx==0 && DIRy>0) npcArray[idx]->DirLoc=0; //6시방향
    else if(DIRx>0 && DIRy>0) npcArray[idx]->DirLoc=7; //7시방향
    else if(DIRx>0 && DIRy==0) npcArray[idx]->DirLoc=6; //9시방향
    else if(DIRx>0 && DIRy<0) npcArray[idx]->DirLoc=5; //11시방향
    pthread_mutex_unlock(&npclock);

    DaMaGe = npcArray[idx]->AP - chr->defense;
    if(DaMaGe<0) DaMaGe=0;
    HP = chr->hp_c - DaMaGe;
    if(HP<0) HP=0;
    
    mesg[rLen] = PK_OBJ_UPDATE_ACT;
    rLen+=1;
    mesg[rLen] = T_NPC;
    rLen+=1;
    mesg[rLen] = 1;
    rLen+=1;
    mesg[rLen] = npcArray[idx]->div;
    rLen+=1;
    memcpy(&mesg[rLen], &npcArray[idx]->npc_id, 2);
    rLen+=2;
    mesg[rLen] = S_FIGHT;
    rLen+=1;
    mesg[rLen] = T_USER;
    rLen+=1;
    memcpy(&mesg[rLen], &chr->userNo, 2);
    rLen+=2;
    memcpy(&mesg[rLen], &HP, 2);
    rLen+=2;
    memcpy(&mesg[rLen], &nEnd, 2);
    rLen+=2;
    memcpy(&mesg[0], &rLen, 2);
    map_usersend_All(chr->Ax, chr->Ay, mesg, rLen, NULL);

    pthread_mutex_lock(&synclock);
    if(HP==0) { //캐릭터 죽음
        chr->userStat = S_DIE;
        chr->hp_c = chr->hp_m/10;
        pthread_mutex_unlock(&synclock);
        pthread_mutex_lock(&npclock);

        npcArray[idx]->status = S_STAND;
        npcArray[idx]->targetChr = NULL;
        npcArray[idx]->tgFlag = 0;

        return;
    }else chr->hp_c = HP; 
    pthread_mutex_unlock(&synclock);
    pthread_mutex_unlock(&npclock);
    npcArray[idx]->tgFlag = 1;
}

extern type_npc * npc_ret_fight(unsigned short idx){
    return NULL;
}