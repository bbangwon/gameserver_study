#include <stdlib.h>
#include <string.h>

#include "skill.h"

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