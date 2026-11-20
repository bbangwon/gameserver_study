#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "que.h"
#include "map.h"
#include "packet.h"
#include "publicInc.h"

static type_map	mapArray[S_WIDTH][S_WIDTH];

// 매개변수로 받는 i, j 좌표 섹터의 객체 리스트 중에서 첫번째 객체를 반환하고, 다음 번 객체를 지정해 놓는다.
extern void *voidlist_get_first(type_objlist const * const **save, unsigned short i, unsigned short j){
    void *conn;

    if(!save)
        return NULL;

    *save = (type_objlist const * const *)&(mapArray[i][j].objL);
    if(!**save)
    {
        *save = NULL;
        return NULL;
    }
    conn = get_node_objlist(**save);
    *save = get_next_node_objlist_const(**save);
    return conn;
}

//매개변수에 지정된 객체 리스트의 첫번째 객체를 리턴한다. 매개변수에 다음 번 객체를 지정한다.
extern void *voidlist_get_next(type_objlist const * const **save) {
    void *conn;

    if(!save)
        return NULL;

    if(!*save || !**save)
    {
        *save = NULL;
        return NULL;
    }

    conn = get_node_objlist(**save);
    *save = get_next_node_objlist_const(**save);
    return conn;
}

// 맵 섹터의 객체 리스트에 새로운 캐릭터 객체를 추가한다. 
// 초기화된 캐릭터에는 위치 값이 있으므로 캐릭터의 위치에 해당하는 섹터의 객체 리스트에 캐릭터 객체를 추가한다.
extern void mapinit(type_session *c){
    add_node_objlist(&(mapArray[c->Ax][c->Az].objL), c);
}

// 맵 섹터의 객체 리스트에 캐릭터 객체를 찾아 삭제한다. 보통 서버와 연결이 끊어질 경우에 사용한다.
extern void map_char_rm(type_session *c) {
    type_objlist **pos;
    pos = find_node_objlist(&(mapArray[c->Ax][c->Az].objL), c);
    // 맵 섹터에서 노드 찾기
    if(pos == NULL)
        return;
    rm_objlist(pos);
}

// 매개변수로 받은 섹터 좌표로 이동이 가능한지를 체크
// 현재 섹터의 객체 리스트에서 캐릭터 노드를 찾아 떼어내어 목적지 섹터의 객체 리스트에 추가
// 이동이 성공했으므로 새롭게 적용되는 섹터 범위의 정보를 얻고, 섹터 유저에게 옮겨왔음을 알린다.
extern int map_char_one_mv(type_session *c, unsigned short x, unsigned short z) {
    type_objlist **pos;
    type_objlist *tmp;
    short tx, tz;

    if(x>=S_WIDTH || z >= S_WIDTH) { // 섹터 좌표가 유효하지 않음
        printf("sector invalid %d, %d\n",x,z);
        return 0;
    }
    if(x==c->Ax&&z==c->Az){
        return 0; // 이미 같은 섹터에 있으므로 이동할 필요 없음
    }
    tx = c->Ax - x;
    tz = c->Az - z;
    if(tx>1||tx<-1||tz>1||tz<-1) {
        printf("이동 좌표 오류\n");
        return 0; // 이동 좌표가 유효하지 않음
    }
    if(mapArray[c->Ax][c->Az].objL == NULL){
        printf("이전 섹터 오류\n");
        return 0;
    }
    pos = find_node_objlist(&(mapArray[c->Ax][c->Az].objL),c);
    if(pos==NULL){ // 노드를 찾지 못함
        printf("pos NULL\n");
        return 0;
    }
    tmp = pop_data(pos);
    if(!tmp) {  // pop_data 실패
        printf("pop NULL\n");
        return 0;
    }
    insert_node_objlist(&(mapArray[x][z].objL), tmp);
    c->Ax=x; // 캐릭터의 새로운 섹터 좌표 적용
    c->Az=z; // 캐릭터의 새로운 섹터 좌표 적용
    map_bk_sectorinfo(c);
    map_get_sectorinfo(c);
    return 1;
}

//매개변수로 받는 섹터 좌표를 중심으로 3x3 섹터의 캐릭터에게 패킷을 전송한다. 단 게임 중인 캐릭터에게만 전송한다.
extern void map_usersend_All(unsigned short mapSizeX, unsigned short mapSizeZ, char *retdata, int len, type_session *c) {
    // 좌표x, y는 배열 주소를 받는다. chr이 NULL일 경우 모두에게 보낸다.
    // chr이 있을경우 chr은 빼고 보낸다.
    int i,j,x,z;
    type_objlist const * const * tmp_objlist;
    void *tmp_char;
    void *ntmp_char;
    type_session *tc;
    type_packet *relaypacket;
    pthread_mutex_lock(&sendQuelock);
    for(i=3;i>0;i--) {
        x = mapSizeX -i+2;
        if(x<0 ||x>S_WIDTH-1)
            continue;
        for(j=3;j>0;j--) {
            z = mapSizeZ -j+2;
            if(z<0||z>S_WIDTH-1)
                continue;

            for(tmp_char=voidlist_get_first(&tmp_objlist, x, z); tmp_char; tmp_char=ntmp_char){
                ntmp_char = voidlist_get_next(&tmp_objlist);
                tc = (type_session *)tmp_char;
                if(tc->state==conn_state_destroy)
                    continue;

                if(c==NULL){
                    if(!(relaypacket = packet_create())){
                        pthread_mutex_unlock(&sendQuelock);
                        return;
                    }
                    memset(relaypacket->data, 0, MAX_PACKET_SIZE);
                    relaypacket->size = len;
                    relaypacket->cli_sock =tc->sock;
                    relaypacket->session = tc;
                    memcpy(&relaypacket->data[0], &retdata[0], len);
                    queue_push_packet_raw(&send_que, relaypacket);
                } else {
                    if(c!=tc) {
                        if(!(relaypacket = packet_create())) {
                            pthread_mutex_unlock(&sendQuelock);
                            return;
                        }
                        memset(relaypacket->data, 0, MAX_PACKET_SIZE);
                        relaypacket->size = len;
                        relaypacket->cli_sock = tc->sock;
                        relaypacket->session = tc;
                        memcpy(&relaypacket->data[0], &retdata[0], len);
                        queue_push_packet_raw(&send_que, relaypacket);
                    }
                }
            }
        }
    }
    pthread_cond_signal(&sendQuecond);
    pthread_mutex_unlock(&sendQuelock);

}

//옮기기 전의 섹터와 새로 옮긴 섹터의 방향 정보를 알아내어 방향에 따라 새로 적용되는 섹터로 해당 유저의 동작 패킷을 보내준다.
void map_bk_sectorinfo(type_session *c){
    int x,z;
    int ax,az,i;
    short dLen=2;
    char udata[128];
    unsigned short nEnd = PEND;

    memset(udata, 0, sizeof(udata));
    udata[dLen] = PK_USER_MV;   //2
    dLen+=1;

    udata[dLen] = RET_TRUE;
    dLen+=1;

    memcpy(&udata[dLen], &c->userNo, 2);
    dLen+=2;

    udata[dLen] = c->uLen;
    dLen+=1;

    memcpy(&udata[dLen], &c->userid[0], c->uLen);
    dLen+=c->uLen;

    udata[dLen] = c->Ax;
    dLen+=1;

    udata[dLen] = c->Az;
    dLen+=1;

    udata[dLen] = c->Bx;
    dLen+=1;

    udata[dLen] = c->Bz;
    dLen+=1;

    memcpy(&udata[dLen], &nEnd, 2);
    dLen+=2;

    memcpy(&udata[0], &dLen, 2);

    x = c->Bx - c->Ax;
    z = c->Bz - c->Az;

    if(x==0 && z<0) { //12시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }        
    } else if(x<0 && z<0) { // 1시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }

        for (i=0;i<2;i++) {
            ax = c->Ax +1;
            az = c->Az -i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x<0 && z==0) { //3시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax +1;
            az = c->Az -i+2;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x<0 && z>0) { //4시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            az = c->Az +i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x==0 && z>0) { // 6시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x>0 && z>0) { // 7시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            az = c->Az +i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x>0 && z==0) { // 9시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -1;
            az = c->Az -i+2;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    } else if(x>0 && z<0) { // 10시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            az = c->Az -i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd(ax, az, udata, dLen);
            } else continue;
        }
    }

}

// 옮기기 전의 섹터와 새로 옮긴 섹터를 통해 방향 정보를 알아내어 방향에 따라 새로운 섹터 정보를 해당 유저에게 보낸다.
void map_get_sectorinfo(type_session *c){
    int x,z,ax,az,i;

    x = c->Bx - c->Ax;
    z = c->Bz - c->Az;

    if(x==0 && z<0) {   // 12시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x<0 && z<0) { // 1시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
        
        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            az = c->Az -i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x<0 && z==0) { // 3시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax +1;
            az = c->Az -i+2;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x<0 && z>0) { // 4시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
        for(i=0;i<2;i++) {
            ax = c->Ax +1;
            az = c->Az +i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x==0 && z>0) { // 6시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x>0 && z>0) { // 7시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az -1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            az = c->Az +i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x>0 && z==0) { // 9시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -1;
            az = c->Az -i+2;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    } else if(x>0 && z<0) { // 10시 방향
        for(i=3;i>0;i--) {
            ax = c->Ax -i+2;
            az = c->Az +1;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }

        for(i=0;i<2;i++) {
            ax = c->Ax -1;
            az = c->Az -i;

            if(ax>=0 && ax<S_WIDTH && az>=0 && az<S_WIDTH) {
                map_secdata_snd_client(ax, az, c);
            } else continue;
        }
    }    
}

//서버와의 연결이 끊어지면 주위 유저에게 연결 종료를 알려야 한다. 
//주위 캐릭터의 경우에는 연결이 끊어진 유저의 정보를 더이상 갖고 있을 필요가 없고, 동기를 맞추어야 하기 때문이다.
extern void map_userDisconn(type_session *c)
{
    char data[16];
    short dLen = 2;
    unsigned short nEnd = PEND;
    printf("map_userDisconn\n");
    
    data[dLen] = PK_USER_DISCON;
    dLen += 1;

    memcpy(&data[dLen], &c->userNo, 2);
    dLen += 2;

    memcpy(&data[dLen], &nEnd, 2);
    dLen += 2;

    memcpy(&data[0], &dLen, 2);
    printf("디스커넥트 패킷보낼 때 유저 인덱스..%d\n", c->userNo);
    map_usersend_All(c->Ax, c->Az, data, dLen, c);
}

//한 섹터의 유저 정보 리스트를 순회하면서 노드의 유저 정보를 클라이언트에게 전송한다.
extern void map_secdata_snd_client(short ax, short az, type_session *c)
{
    type_objlist const * const *tmp_objlist;
    void *tmp_char;
    void *ntmp_char;
    type_session *tc;
    short objCnt=0,dLen=2;
    char udata[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;

    udata[dLen] = PK_USER_INFO;
    dLen += 1;
    // memcpy(&udata[dLen], &objCnt, 2); //유저 카운트는 뒤에서 구한다.
    dLen += 2; // 유저 카운트 자리 확보

    for(tmp_char=voidlist_get_first(&tmp_objlist, ax, az); tmp_char; tmp_char=ntmp_char)
    {
        ntmp_char = voidlist_get_next(&tmp_objlist);
        tc=(type_session *)tmp_char;
        objCnt+=1;
        
        memcpy(&udata[dLen], &tc->userNo, 2);
        dLen += 2;

        udata[dLen] = tc->uLen;
        dLen += 1;

        memcpy(&udata[dLen], &tc->userid[0], tc->uLen);
        dLen += tc->uLen;

        udata[dLen] = tc->Ax;
        dLen += 1;

        udata[dLen] = tc->Az;
        dLen += 1;
        
        if(dLen > MAX_PACKET_SIZE-40) {
            memcpy(&udata[dLen], &nEnd, 2);
            dLen += 2;

            memcpy(&udata[0], &dLen, 2);
            memcpy(&udata[3], &objCnt, 2); // 유저 카운트 기록
            map_pData_snd(c, udata, dLen);
            objCnt = 0;
            dLen = 5;
        }
    }

    if(objCnt>0){
        memcpy(&udata[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&udata[0], &dLen, 2);
        memcpy(&udata[3], &objCnt, 2); // 유저 카운트 기록
        map_pData_snd(c, udata, dLen);
    }
}

// 섹터의 유저 정보 리스트를 순회하면서 매개변수로 받은 데이터를 노드의 유저에게 전송한다.
// 전송할 패킷을 생성하고 패킷을 구성하는 정보를 설정하여 패킷 송신 큐에 넣는다.
void map_secdata_snd(short ax, short az, char *retdata, short dLen)
{
    type_objlist const * const *tmp_objlist;
    void *tmp_char;
    void *ntmp_char;
    type_session *tc;
    type_packet *relaypacket;
    // printf("map_secdata_snd\n");
    pthread_mutex_lock(&sendQuelock);
    for(tmp_char=voidlist_get_first(&tmp_objlist, ax, az); tmp_char; tmp_char=ntmp_char)
    {
        ntmp_char = voidlist_get_next(&tmp_objlist);
        tc = (type_session *)tmp_char;

        if(!(relaypacket = packet_create())) {
            pthread_mutex_unlock(&sendQuelock);
            return;
        }

        memset(relaypacket->data, 0, MAX_PACKET_SIZE);
        relaypacket->size = dLen;
        relaypacket->cli_sock = tc->sock;
        relaypacket->session = tc;
        memcpy(&relaypacket->data[0], &retdata[0], dLen);
        queue_push_packet_raw(&send_que, relaypacket);
        // sendQuedone++;
    }
    pthread_cond_signal(&sendQuecond);
    pthread_mutex_unlock(&sendQuelock);
}

// 3x3 섹터를 순회하는 방식으로 섹터 유저의 정보를 받아 해당 클라이언트에게 전송한다.
// 유저가 맵에 처음으로 위치할 때 유저의 정보를 받게 하는 함수다. 
// 현재 위치하는 섹터를 중심으로 차례대로 섹터에서 유저 정보를 패킷으로 만들어 해당 유저에게 전송한다.
extern void map_get_sectorinfo_All(type_session *c)
{
    int i,j,x,z;
    type_objlist const * const *tmp_objlist;
    void *tmp_char;
    void *ntmp_char;
    type_session *tc;
    short objCnt=0, dLen=2;
    char udata[MAX_PACKET_SIZE];
    unsigned short nEnd = PEND;

    udata[dLen] = PK_USER_INFO;
    dLen += 1;
    // memcpy(&udata[dLen], &objCnt, 2); // 유저 카운트 기록
    dLen += 2; // objCnt 자리 확보

    for(i=3;i>0;i--) {
        x = c->Ax -i+2;
        if(x<0 || x>S_WIDTH-1) continue;

        for(j=3;j>0;j--) {
            z = c->Az -j+2;
            if(z<0 || z>S_WIDTH-1) continue;

            for(tmp_char=voidlist_get_first(&tmp_objlist, x, z); tmp_char; tmp_char=ntmp_char) {
                ntmp_char = voidlist_get_next(&tmp_objlist);
                tc = (type_session *)tmp_char;
                if(tc->userNo == c->userNo) continue; // 자기 자신은 제외
                objCnt+=1;

                memcpy(&udata[dLen], &tc->userNo, 2);
                dLen += 2;

                udata[dLen] = tc->uLen;
                dLen += 1;

                memcpy(&udata[dLen], &tc->userid[0], tc->uLen);
                dLen+=tc->uLen;

                udata[dLen] = tc->Ax;
                dLen += 1;

                udata[dLen] = tc->Az;
                dLen += 1;

                if(dLen>MAX_PACKET_SIZE-40)
                {
                    memcpy(&udata[0], &dLen, 2);
                    memcpy(&udata[3], &objCnt, 2); // objCnt 기록
                    map_pData_snd(c, udata, dLen);
                    objCnt = 0;
                    dLen = 5;
                }                
            }
        }
    }

    if(objCnt > 0){
        memcpy(&udata[dLen], &nEnd, 2);
        dLen += 2;
        memcpy(&udata[0], &dLen, 2);
        memcpy(&udata[3], &objCnt, 2); // objCnt 기록
        map_pData_snd(c, udata, dLen);
    }
}

void map_pData_snd(type_session *c, char *mesg, int pLen) {
}



