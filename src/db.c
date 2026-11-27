#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "item.h"
#include "npc.h"
#include "skill.h"
#include "publicInc.h"

static int exp_list[MAXUSERLEVEL+1];

// 아이템 리스트를 데이터베이스에서 로드하는 함수
extern void db_load_itemList(void *con)
{
    int i;
    PGconn *conn;
    PGresult *result;

    char query[256];
    int rows;

    conn = (PGconn *)con;
    item_init();
    memset(query, 0, 256);
    sprintf(query, "SELECT * FROM ITEM");

    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("db_load_itemList:%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return;
    }

    rows = PQntuples(result);
    if(rows == 0) {
        printf("db no item list found0\n");
        PQclear(result);
        return;
    }

    for(i = 0; i < rows; i++) {
        type_item item;
        item.is_init = 1;
        item.idx_type = (char)atoi(PQgetvalue(result, i, 0));
        item.item_no = (unsigned short)atoi(PQgetvalue(result, i, 1));        
        item.equip = (char)atoi(PQgetvalue(result, i, 2));
        item.item_type = (char)atoi(PQgetvalue(result, i, 3));
        item.req = (short)atoi(PQgetvalue(result, i, 4));
        item.pwr = (unsigned short)atoi(PQgetvalue(result, i, 5));
        item.range = (char)atoi(PQgetvalue(result, i, 6));
        item.itemclass = (char)atoi(PQgetvalue(result, i, 7));
        item.amount = (unsigned short)atoi(PQgetvalue(result, i, 8));
        item.coin = (unsigned short)atoi(PQgetvalue(result, i, 9));
        item.isbuy = 0;

        item_load(&item);
    }
    printf("아이템 총 %d 개 로드됨\n", rows);
    PQclear(result);
}

extern void db_load_npcList(void *con)
{
    PGconn *conn;
    PGresult *result;

    char query[256];
    char div, level, gtype, speed;
    short no;
    int rows;

    conn = (PGconn *)con;
    item_init();
    memset(query, 0, 256);
    sprintf(query, "SELECT * FROM npc");
    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("db_load_npcList:%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return;
    }

    rows = PQntuples(result);
    if(rows == 0) {
        printf("db no npc list found0\n");
        PQclear(result);
        return;
    }

    for(int i = 0; i < rows; i++) {
        div = (char)atoi(PQgetvalue(result, i, 0));
        no = (short)atoi(PQgetvalue(result, i, 1));
        gtype = (char)atoi(PQgetvalue(result, i, 2));
        level = (char)atoi(PQgetvalue(result, i, 3));
        speed = (char)atoi(PQgetvalue(result, i, 4));
        npc_set_def(div, no, gtype, level, speed);
        if(div == 0)
            printf("NPC load no:%d gtpye:%d ;eve;:%d speed:%d\n", no, gtype, level, speed);
        else
            printf("MONSTER load no:%d gtpye:%d ;eve;:%d speed:%d\n", no, gtype, level, speed);
    }
    printf("NPC 총 %d 개 로드됨\n", rows);
    PQclear(result);
}

extern void db_load_expList(void *con)
{
    int lv;

    PGconn *conn;
    PGresult *result;
    char query[256];
    int rows;

    conn = (PGconn *)con;
    item_init();
    memset(query, 0, 256);
    sprintf(query, "SELECT * FROM exp ORDER BY lv ASC");

    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("db_load_expList:%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return;
    }

    rows = PQntuples(result);
    if(rows == 0) {
        printf("db no exp list found0\n");
        PQclear(result);
        return;
    }

    for(int i = 0; i < rows; i++) {
        lv = atoi(PQgetvalue(result, i, 0));
        exp_list[lv]=atoi(PQgetvalue(result, i, 1));
        printf("레벨%d는 총 %d의 경험치가 있어야 %d로 레벨업 됨\n", lv, exp_list[lv], lv+1);
    }
    printf("EXP 총 %d 개 로드됨\n", rows);
    PQclear(result);
}

extern void db_load_skillList(void *con)
{
    int sktype, coin, skidx, i;
    char *lvpwr, *lvupcost;
    PGconn *conn;
    PGresult *result;
    char query[256];
    int rows;


    conn = (PGconn *)con;
    item_init();
    memset(query, 0, 256);
    sprintf(query, "SELECT * FROM skilldef");

    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("db_load_skillList:%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return;
    }

    rows = PQntuples(result);
    if(rows == 0) {
        printf("db no skill list found0\n");
        PQclear(result);
        return;
    }

    for(i = 0; i < rows; i++) {
        sktype = atoi(PQgetvalue(result, i, 0));
        skidx = atoi(PQgetvalue(result, i, 1));
        coin = atoi(PQgetvalue(result, i, 2));
        lvpwr = PQgetvalue(result, i, 3);
        lvupcost = PQgetvalue(result, i, 4);
        skill_defset(sktype, skidx, lvpwr, lvupcost, coin);
    }
    printf("Skill 총 %d 개 로드됨\n", rows);
    PQclear(result);
}

extern int db_user_auth(char *userid, char *passwd, int passlen, void *conn, type_session *c)
{
    int state, len;
    PGresult *result;
    char query[256];

    memset(query, 0, 256);
    sprintf(query, "SELECT passwd from userinfo WHERE userid='%s'", userid);
    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return 0;
    }
    if(PQntuples(result) == 0) {
        printf("pgsql fail return value 0\n");
        PQclear(result);
        return 0;
    }
    len = strlen(PQgetvalue(result, 0, 0));
    if(passlen==len) {
        if(!strncmp(PQgetvalue(result, 0, 0), passwd, len)) { //인증 성공
            PQclear(result);
            memset(query, 0, 256);
            sprintf(query, "SELECT * FROM character WHERE userid='%s'", userid);

            result = PQexec((PGconn *)conn, query);
            if(PQresultStatus(result) != PGRES_TUPLES_OK) {
                printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
                PQclear(result);
                return 0;
            }
            if(PQntuples(result) == 0) { //아이디에 해당하는 캐릭터가 없는 경우
                printf("pgsql fail return value 0\n");
                PQclear(result);
                return 0;
            }
            c->nLen = strlen(PQgetvalue(result, 0, 1)); // 캐릭터 이름 길이
            memcpy(&c->char_name[0], PQgetvalue(result, 0, 1), c->nLen); // NAME
            c->race = atoi(PQgetvalue(result, 0, 2)); // RACE
            c->sex = atoi(PQgetvalue(result, 0, 3)); // SEX
            c->nation = atoi(PQgetvalue(result, 0, 4)); // NATION

            c->str = atoi(PQgetvalue(result, 0, 5)); // STR
            c->dex = atoi(PQgetvalue(result, 0, 6)); // DEX
            c->intel = atoi(PQgetvalue(result, 0, 7)); // INTEL

            c->mana_c = atoi(PQgetvalue(result, 0, 8)); // MANA_C
            c->hp_c = atoi(PQgetvalue(result, 0, 9)); // HP_C
            c->exp = atoi(PQgetvalue(result, 0, 10)); // EXP
            c->level = atoi(PQgetvalue(result, 0, 11)); // LEVEL
            c->lvpoint = atoi(PQgetvalue(result, 0, 12)); // LVPOINT
            c->skexp = atoi(PQgetvalue(result, 0, 13)); // SKEXP

            c->jobno = atoi(PQgetvalue(result, 0, 14)); // JOBNO
            c->classno = atoi(PQgetvalue(result, 0, 15)); // CLASSNO
            c->classlevel = atoi(PQgetvalue(result, 0, 16)); // CLASSLEVEL
            c->worldmap = atoi(PQgetvalue(result, 0, 17)); // WORLDMAP
            c->Cx = atoi(PQgetvalue(result, 0, 18)); // Cx
            c->Cy = atoi(PQgetvalue(result, 0, 19)); // Cy
            c->Ax = c->Cx/S_UNITSIZE;
            c->Ay = c->Cy/S_UNITSIZE;
            c->Bx = c->Ax;
            c->By = c->Ay;

            c->eq[0] = atoi(PQgetvalue(result, 0, 20)); // I_HEAD
            c->eq[1] = atoi(PQgetvalue(result, 0, 21)); // I_L_HAND
            c->eq[2] = atoi(PQgetvalue(result, 0, 22)); // I_R_HAND
            c->eq[3] = atoi(PQgetvalue(result, 0, 23)); // I_BODY
            c->eq[4] = atoi(PQgetvalue(result, 0, 24)); // I_LEG
            c->eq[5] = atoi(PQgetvalue(result, 0, 25)); // I_NECKLACE
            c->eq[6] = atoi(PQgetvalue(result, 0, 26)); // I_RING
            c->coin = atoi(PQgetvalue(result, 0, 27)); // COIN
            PQclear(result);

            memset(query, 0, 256);
            sprintf(query, "SELECT * FROM USERITEM WHERE userid='%s'", userid); //인벤토리
            result = PQexec((PGconn *)conn, query);
            if(PQresultStatus(result) != PGRES_TUPLES_OK) {
                printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
                PQclear(result);
                return 0;
            }
            if(PQntuples(result) == 0) { //아이템이 없는 경우
                printf("pgsql fail return value 0\n");
                PQclear(result);
                return 0;
            }
            c->inven[0] = atoi(PQgetvalue(result, 0, 2)); // 첫 번째 인벤토리 아이템
            c->inven_cnt[0] = atoi(PQgetvalue(result, 0, 3)); // 첫 번째 인벤토리 아이템 개수
            c->inven[1] = atoi(PQgetvalue(result, 0, 4)); // 두 번째 인벤토리 아이템
            c->inven_cnt[1] = atoi(PQgetvalue(result, 0, 5)); // 두 번째 인벤토리 아이템 개수
            c->inven[2] = atoi(PQgetvalue(result, 0, 6)); // 세 번째 인벤토리 아이템
            c->inven_cnt[2] = atoi(PQgetvalue(result, 0, 7)); // 세 번째 인벤토리 아이템 개수
            c->inven[3] = atoi(PQgetvalue(result, 0, 8)); // 네 번째 인벤토리 아이템
            c->inven_cnt[3] = atoi(PQgetvalue(result, 0, 9)); // 네 번째 인벤토리 아이템 개수
            PQclear(result);

            memset(query, 0, 256);
            sprintf(query, "SELECT * FROM SKILL WHERE userid='%s'", userid); //스킬
            result = PQexec((PGconn *)conn, query);
            if(PQresultStatus(result) != PGRES_TUPLES_OK) {
                printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
                PQclear(result);
                return 0;
            }
            if(PQntuples(result) == 0) { //스킬이 없는 경우
                printf("pgsql fail return value 0\n");
                PQclear(result);
                return 0;
            }
            c->skill[0] = atoi(PQgetvalue(result, 0, 2)); // 첫 번째 스킬
            c->skill_lv[0] = atoi(PQgetvalue(result, 0, 3)); // 첫 번째 스킬 레벨
            c->skill[1] = atoi(PQgetvalue(result, 0, 4)); // 두 번째 스킬
            c->skill_lv[1] = atoi(PQgetvalue(result, 0, 5)); // 두 번째 스킬 레벨
            c->skill[2] = atoi(PQgetvalue(result, 0, 6)); // 세 번째 스킬
            c->skill_lv[2] = atoi(PQgetvalue(result, 0, 7)); // 세 번째 스킬 레벨
            c->skill[3] = atoi(PQgetvalue(result, 0, 8)); // 네 번째 스킬
            c->skill_lv[3] = atoi(PQgetvalue(result, 0, 9)); // 네 번째 스킬 레벨
            PQclear(result);

            memset(query, 0, 256);
            sprintf(query, "SELECT * FROM WAREHOUSE WHERE userid='%s'", userid);
            result = PQexec((PGconn *)conn, query);
            if(PQresultStatus(result) != PGRES_TUPLES_OK) {
                printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
                PQclear(result);
                return 0;
            }
            if(PQntuples(result) == 0) { //창고가 없는 경우
                printf("pgsql fail return value 0\n");
                PQclear(result);
                return 0;
            }
            c->ware[0] = atoi(PQgetvalue(result, 0, 1)); // 첫 번째 창고 아이템
            c->ware_cnt[0] = atoi(PQgetvalue(result, 0, 2)); // 첫 번째 창고 아이템 개수
            c->ware[1] = atoi(PQgetvalue(result, 0, 3)); // 두 번째 창고 아이템
            c->ware_cnt[1] = atoi(PQgetvalue(result, 0, 4)); // 두 번째 창고 아이템 개수
            c->ware[2] = atoi(PQgetvalue(result, 0, 5)); // 세 번째 창고 아이템
            c->ware_cnt[2] = atoi(PQgetvalue(result, 0, 6)); // 세 번째 창고 아이템 개수
            c->ware[3] = atoi(PQgetvalue(result, 0, 7)); // 네 번째 창고 아이템
            c->ware_cnt[3] = atoi(PQgetvalue(result, 0, 8)); // 네 번째 창고 아이템 개수
            c->ware[4] = atoi(PQgetvalue(result, 0, 9)); // 다섯 번째 창고 아이템
            c->ware_cnt[4] = atoi(PQgetvalue(result, 0, 10)); // 다섯 번째 창고 아이템 개수
            c->ware[5] = atoi(PQgetvalue(result, 0, 11)); // 여섯 번째 창고 아이템
            c->ware_cnt[5] = atoi(PQgetvalue(result, 0, 12)); // 여섯 번째 창고 아이템 개수
            c->ware[6] = atoi(PQgetvalue(result, 0, 13)); // 일곱 번째 창고 아이템
            c->ware_cnt[6] = atoi(PQgetvalue(result, 0, 14)); // 일곱 번째 창고 아이템 개수
            c->ware[7] = atoi(PQgetvalue(result, 0, 15)); // 여덟 번째 창고 아이템
            c->ware_cnt[7] = atoi(PQgetvalue(result, 0, 16)); // 여덟 번째 창고 아이템 개수
            PQclear(result);

            c->mana_F_cnt = 0; //마나 회복용 카운트
            c->recov_F_cnt = 0; //히어로 회복용 카운트
            c->attack_F_cnt = 0; //히어로 공격 지연용 카운트
            c->regen_F_cnt = 0; //리젠 카운트
            c->dbSaveCnt = 0; //데이터 저장 카운트
            c->move_F_cnt = 0; 

            c->party.flag = 0; //파티 플래그 초기화
            c->party.waitfor = NULL; //기다리는 사람 초기화
            c->party.cnt = 0; //파티 인원 초기화
            c->party.memidx = 0; //자신의 위치
            c->party.infoFcnt = 0; //유저 정보 알림 카운트
            c->party.ison1 = 0; //0: NULL, 1: linked, 2: myself(member#==NULL)
            c->party.ison2 = 0; 
            c->party.ison3 = 0; 
            c->trade.flag = 0; //거래 플래그 초기화
            c->moveLevel = 2;
            c->pkmode = 0; // Pk 모드는 기본적으로 켜 있다.

            c->attack = c->str+c->dex+c->intel+c->level+item_get_attack(c->eq[1])+item_get_attack(c->eq[2]);
            c->hp_m = c->str+c->dex+50;
            c->mana_m = c->intel+50;
            c->defense = item_get_defense(c->eq[0]) + 
                        item_get_defense(c->eq[1]) + 
                        item_get_defense(c->eq[2]) +
                        item_get_defense(c->eq[3]) +
                        item_get_defense(c->eq[4]) +
                        item_get_defense(c->eq[5]) +
                        item_get_defense(c->eq[6]);
            c->dbLoaded = 1;
            return 1;
        }else{
            return 0;
        }
    } else {
        return 0;
    }
}

extern int db_chk_lvup(void * cc,void * con)
{
    type_session *c;
    PGconn *conn;
    // char query[256];
    // PGresult *result;
    
    c=(type_session *)cc;
    conn=(PGconn *)con;
    if(c->level > MAXUSERLEVEL || c->level==0) return 0;
    if(exp_list[c->level]<=c->exp) {
        c->level+=1;
        c->lvpoint+=1;
        // sprintf(query, "UPDATE characters SET LV='%d', PT='%d' WHERE userid='%s'", c->level, c->lvpoint, c->userid);
        // result = PQexec(conn, query);
        // if(PQresultStatus(result) != PGRES_COMMAND_OK) {
        //     printf("update lv & lvpoint error:%s\n", PQerrorMessage(conn));
        //     PQclear(result);
        //     return 0;
        // }
        // PQclear(result);
        return 1;       
    }else return 0;

}