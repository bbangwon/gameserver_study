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

void map_bk_sectorinfo(type_session *c){

}

void map_get_sectorinfo(type_session *c){

}