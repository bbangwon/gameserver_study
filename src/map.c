#include <stdio.h>
#include <stdlib.h>
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
    // Implementation for sending the current sector info to the client
}

extern void map_get_sectorinfo(type_session *c) {
    // Implementation for receiving the current sector info from the client
}