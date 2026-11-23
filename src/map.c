#include <stdio.h>
#include <stdlib.h>
#include "map.h"
#include "npc.h"

#define MAP_FILE "/home/bbangwon/workspace/gameserver_study/svrmap1.dat"
#define M_SIZE_X 512

static char			map[M_SIZE_X][M_SIZE_X];

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