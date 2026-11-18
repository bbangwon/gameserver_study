#include <stdio.h>
#include <stdlib.h>
#include "packet.h"

// 패킷 노드에 메모리를 할당하여 리턴
extern type_packet *packet_create() {
    type_packet *temp;
    if(!(temp = malloc(sizeof(type_packet)))){
        printf("패킷 생성 메모리 할당 오류\n");
        return NULL;
    }
    return temp;
}

// 패킷 노드의 메모리를 해제한다.
extern void packet_destroy(type_packet *packet){
    if(packet)
        free((void *)packet);
}
