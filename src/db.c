#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "db.h"
#include "item.h"
#include "npc.h"

// 아이템 리스트를 데이터베이스에서 로드하는 함수
extern void db_load_itemList(void *con)
{
    int i, state, cnt = 0;
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
    int state;
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
    PQclear(result);
}