#pragma once

#include "packet.h"

 //송신할 패킷을 큐 구조로 저장
 typedef struct queues {
    type_packet *packet;
    struct queues *next;
 } type_queue;

 type_packet* queue_pull_packet(type_queue **queue);
 void queue_push_packet_new(type_queue **queue, type_packet *packet);
 int queue_get_length(type_queue const * const * queue);