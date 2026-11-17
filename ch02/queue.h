#pragma once

#include "packet.h"

 //송신할 패킷을 큐 구조로 저장
 typedef struct queues {
    type_packet *packet;
    struct queues *next;
 } type_queue;