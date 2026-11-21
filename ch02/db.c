#include <libpq-fe.h>
#include <string.h>
#include "db.h"

// 유저 아이디와 비밀번호를 사용하는 유저가 있는지 체크. 유저 아이디를 조건으로 해당 유저의 비밀번호를 알아냄.
// 유저가 없거나 비밀번호가 틀리다면 인증 실패
extern int db_user_auth(char *userid, char *passwd, int passlen, void *conn){
    int len;
    PGresult *result;
    char query[256];
    char db_passwd[256];

    memset(query, 0, sizeof(query));
    sprintf(query, "select passwd from userinfo where userid='%s'", userid);

    result = PQexec((PGconn *)conn, query);
    if(PQresultStatus(result) != PGRES_TUPLES_OK) {
        printf("testXX%s\n", PQerrorMessage((PGconn *)conn));
        PQclear(result);
        return 0;
    }

    strcpy(db_passwd, PQgetvalue(result, 0, 0));
    PQclear(result);

    len = strlen(db_passwd);
    if(passlen==len) {
        if(!strncmp(passwd, db_passwd, len)) {            
            return 1;
        } else {
            return 0;
        }        
    } else {
        return 0;
    }   
}