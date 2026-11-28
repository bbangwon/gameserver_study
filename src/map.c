#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "map.h"
#include "npc.h"
#include "objlist.h"
#include "item.h"
#include "publicInc.h"

#define MAP_FILE "/home/bbangwon/workspace/gameserver_study/svrmap1.dat"

static type_map	mapArray[S_WIDTH][S_WIDTH];
static char			map[M_SIZE_X][M_SIZE_X];

extern void mapinit(type_session *c) {
    add_node_objlist(&(mapArray[c->Ax][c->Ay].objL), c);
}   

extern void map_Load(){
    FILE *fp;
    unsigned int mxtileno;
    unsigned int i,x,y;
    char val;

    fp = fopen(MAP_FILE, "r");
    if (fp == NULL) {
        printf("file Open error\n");
        exit(1);
    }

    fread(&mxtileno, 4, 1, fp); //셀의 개수를 알아낸다.
    for(i=0; i<mxtileno; i++) { //셀의 데이터를 읽어온다.
        fread(&val,1,1,fp); //0: 사용, 1: 몬스터, 2: NPC, 3: 사용안함
        x = i % M_SIZE_X;
        y = i / M_SIZE_X;
        if(val ==0) {
            map[x][y] = 0;
        }
        else if(val>9&&val<100) {
            npc_set(0, val, x, y);
            map[x][y] = 0;
        }
        else if(val > 99) {
            npc_set(1, val, x, y);
            map[x][y] = 0;
        } 
        else map[x][y] = 1; //갈수 없는 곳
    }
}

extern void map_pData_snd(type_session *c, char *mesg, int pLen) {
}

extern void map_secdata_snd(short ax, short ay, char *retdata, short dLen) {
}

extern void map_secdata_snd_client(short x, short z, type_session *c) {
}

extern void map_get_sectorinfo_All(type_session *c) {
}

extern short map_isgo(unsigned short x, unsigned short y) {
    return 0;
}

// 이동중인 캐릭터가 Thread_sync 스레드에 의해 한 칸을 움직이는 프레임이 되었을 때 캐릭터를 목표 지점 방향으로 한 칸을 옮긴다.
extern void map_chrAction_calc(type_session * c) {
    short tmpx, tmpy, res, Ax, Ay, DIRx, DIRy;
    tmpx = c->Cx;
    tmpy = c->Cy;

    if(c->move_F_cnt>=GAME_FRAME){ //속도에 따라 이동 프레임 조정 가능
        DIRx = c->Cx - c->Dx;
        DIRy = c->Cy - c->Dy;

        if(DIRx==0 && DIRy<0) {
            c->Dir = 4; //12시 방향
            tmpy += 1;        
        } else if(DIRx<0 && DIRy<0) {
            c->Dir=3; //1시 방향
            tmpx += 1;
            tmpy += 1;
        } else if(DIRx<0 && DIRy==0) {
            c->Dir = 2; //3시 방향
            tmpy += 1;
        } else if(DIRx<0 && DIRy>0) {
            c->Dir = 1; //5시 방향
            tmpx += 1;
            tmpy -= 1;
        } else if(DIRx==0 && DIRy>0) {
            c->Dir = 0; //6시 방향
            tmpy -= 1;        
        } else if(DIRx>0 && DIRy>0) {
            c->Dir = 7; //7시 방향
            tmpx -= 1;
            tmpy -= 1;
        } else if(DIRx>0 && DIRy==0) {
            c->Dir = 6; //9시 방향
            tmpx -= 1;
        } else if(DIRx>0 && DIRy<0) {
            c->Dir = 5; //11시 방향
            tmpx -= 1;
            tmpy += 1;
        }
        if(tmpx<0 || tmpx >= M_SIZE_X || tmpy<0 || tmpy >= M_SIZE_X) return; //바깥쪽에서 멈춘다.
        res = map_isgo(tmpx, tmpy);
        if(res==0) {
            printf("CHR_MOV %s가 %d, %d에서  %d, %d로 이동\n", c->userid, c->Cx, c->Cy, tmpx, tmpy);
            c->Cx=tmpx;
            c->Cy=tmpy;
            Ax=tmpx / S_UNITSIZE;
            Ay=tmpy / S_UNITSIZE;
            if(c->Ax!=Ax || c->Ay!=Ay) {
                map_char_one_mv(c, Ax, Ay);
            }
        }
        c->move_F_cnt = 0;
    } else c->move_F_cnt++;
}

extern int map_char_one_mv(type_session * c,unsigned short x,unsigned short y) {
    //한칸만 이동시킴
    type_objlist **pos;
    type_objlist *tmp;
    short tx,ty;

    if(x>=S_WIDTH || y >= S_WIDTH) {
        return 0;
    }
    if(x==c->Ax && y==c->Ay) {
        return 0;
    }
    tx = c->Ax - x;
    ty = c->Ay - y;
    if(tx>1 || tx<-1 || ty>1 || ty<-1) {
        return 0;
    }
    if(mapArray[c->Ax][c->Ay].objL == NULL) {
        return 0;
    }
    pos = find_node_objlist(&(mapArray[c->Ax][c->Ay].objL),c); //이전 섹터에서 캐릭터 객체를 찾는다.
    if(pos == NULL) {
        return 0;
    }
    tmp = pop_data(pos);
    if(!tmp) {
        return 0;
    }
    insert_node_objlist(&(mapArray[x][y].objL), tmp); //잘라낸 캐릭터 객체를 새로운 섹터의 객체 리스트에 추가
    c->Ax = x;
    c->Ay = y;
    
    map_bk_sectorinfo(c);   //새로운 유저한테 내 정보 보내기
    map_get_sectorinfo(c);   //새로운 섹터 유저로부터 데이터 받기
    item_part_sectorinfo(c);   //새로운 섹터 아이템 리스트 받기
    npc_chr_mv_addinfo(c);   //새로운 섹터 NPC 리스트 받기

    c->Bx = x;
    c->By = y;
    return 1;

}

extern void map_bk_sectorinfo(type_session *c) {
    //캐릭터의 섹터 이동으로 인해 새로운 섹터에 정보를 전달한다.
    int x,y;
    int ax, ay,i;
    short dLen=2;
    char udata[128];
    unsigned short nEnd = PEND;
    type_session *usr;
    type_npc *npc;

    memset(udata, 0, 128);
    udata[dLen] = PK_OBJ_ADD;
    dLen+=1;

    udata[dLen] = T_USER;
    dLen+=1;

    udata[dLen] = 1;
    dLen+=1;

    memcpy(&udata[dLen], &c->userNo, 2);
    dLen += 2;

    udata[dLen] = c->nLen;
    dLen += 1;

    memcpy(&udata[dLen], &c->char_name[0],c->nLen);
    dLen += c->nLen;

    udata[dLen] = c->race;
    dLen += 1;

    udata[dLen] = c->sex;
    dLen += 1;

    udata[dLen]=c->nation;
    dLen += 1;

    memcpy(&udata[dLen], &c->str, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->dex, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->intel, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->hp_c, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->mana_c, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->exp, 4);
    dLen += 4;

    memcpy(&udata[dLen], &c->level, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->lvpoint, 2);
    dLen += 2;

    memcpy(&udata[dLen], &c->skexp, 2);
    dLen += 2;

    udata[dLen] = c->jobno;
    dLen += 1;

    udata[dLen] = c->classno;
    dLen += 1;

    udata[dLen] = c->classlevel;
    dLen += 1;

    memcpy(&udata[dLen], &c->eq[0], 2);
    dLen += 2;
    memcpy(&udata[dLen], &c->eq[1], 2);
    dLen += 2;
    memcpy(&udata[dLen], &c->eq[2], 2);
    dLen += 2;
    memcpy(&udata[dLen], &c->eq[3], 2);
    dLen += 2;
    memcpy(&udata[dLen], &c->eq[4], 2);
    dLen += 2;
    memcpy(&udata[dLen], &c->coin, 4);
    dLen += 4;

    udata[dLen]=c->userStat;
    dLen += 1;
    if(c->userStat == 0) { //멈춤
        udata[dLen]=c->Dir;
        dLen += 1;

        memcpy(&udata[dLen], &c->Ax, 2);
        dLen += 2;
        memcpy(&udata[dLen], &c->Ay, 2);
        dLen += 2;
    } else if(c->userStat == 1) { //이동
        udata[dLen]=c->Dir;
        dLen += 1;

        udata[dLen] = c->moveLevel;
        dLen += 1;

        memcpy(&udata[dLen], &c->Cx, 2);
        dLen += 2;
        memcpy(&udata[dLen], &c->Cy, 2);
        dLen += 2;
        memcpy(&udata[dLen], &c->Dx, 2);
        dLen += 2;        
        memcpy(&udata[dLen], &c->Dy, 2);
        dLen += 2;
    } else if(c->userStat == 2) { //싸움
        udata[dLen]=c->Dir;
        dLen += 1;
        udata[dLen] = c->tgtype;
        dLen += 1;
        if(c->tgtype == 0) { //PK
            usr=(type_session *)c->target;
            memcpy(&udata[dLen], &usr->userNo, 2);
            dLen += 2;
        } else { //몬스터
            npc=(type_npc *)c->target;
            memcpy(&udata[dLen], &npc->npc_id, 2);
            dLen += 2;
        }
        memcpy(&udata[dLen], &c->Ax, 2);
        dLen += 2;
        memcpy(&udata[dLen], &c->Ay, 2);
        dLen += 2;
    } else { //거래, 죽음
        udata[dLen] = c->Dir;
        dLen += 1;

        memcpy(&udata[dLen], &c->Ax, 2);
        dLen += 2;
        memcpy(&udata[dLen], &c->Ay, 2);
        dLen += 2;        
    }

    memcpy(&udata[dLen], &nEnd, 2);
    dLen += 2;
    memcpy(&udata[0], &dLen, 2);

    x = c->Bx - c->Ax;
    y = c->By - c->Ay;

    if(x==0 && y<0) { //12시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x<0 && y<0) { //1시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            ay = c->Ay -i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x<0 && y==0) { //3시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax +1;
            ay = c->Ay -i+2;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x<0 && y>0) { //4시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            ay = c->Ay +i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x==0 && y>0) { //6시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x>0 && y>0) { //7시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            ay = c->Ay +i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x>0 && y==0) { //9시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -1;
            ay = c->Ay -i+2;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    } else if(x>0 && y<0) { //10시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            ay = c->Ay -i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd(ax, ay, udata, dLen);
            } else continue;
        }
    }
}

extern void map_get_sectorinfo(type_session *c) {
    // 캐릭터의 섹터 이동으로 인해 새로운 섹터에 정보를 받는다.
    int x,y,ax,ay,i;

    x = c->Bx - c->Ax;
    y = c->By - c->Ay;

    if(x==0 && y<0) { //12시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x<0 && y<0) { //1시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            ay = c->Ay -i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x<0 && y==0) { //3시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax +1;
            ay = c->Ay -i+2;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x<0 && y>0) { //4시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            ay = c->Ay +i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x==0 && y>0) { //6시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x>0 && y>0) { //7시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay -1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            ay = c->Ay +i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x>0 && y==0) { //9시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -1;
            ay = c->Ay -i+2;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    } else if(x>0 && y<0) { //10시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            ay = c->Ay +1;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            ay = c->Ay -i;

            if(ax>=0 && ax<S_WIDTH && ay >= 0 && ay < S_WIDTH) {
                map_secdata_snd_client(ax, ay, c);
            } else continue;
        }
    }
}

extern void map_usersend_All(unsigned short mapSizeX, unsigned short mapSizeZ, char *retdata, int len, type_session *c)
{
}

extern short map_ret_dist(unsigned short x, unsigned short y, unsigned short xx, unsigned short yy){

}

extern void * map_ret_Activ_tg(unsigned short npcx, unsigned short npcy,unsigned short sight){

}

extern short map_ret_chrdist(unsigned short x, unsigned short y,unsigned short idx) {
    return 0;
}

extern type_session * map_ret_chr(unsigned short Ax,unsigned short Ay,unsigned short tidx){
    return NULL;
}

extern void map_party_sendAll(void * chrr,char * data,int len){
}