#include <stdlib.h>
#include "que.h"

// 큐에서 패킷을 꺼냄
extern type_packet *queue_pull_packet(type_queue **queue)
{
    type_queue *temp;
    type_packet *packet;

    if(!queue)
        return NULL;
    if(!*queue)
        return NULL;
    temp = *queue;
    *queue = (*queue)->next;
    packet = temp->packet;
    free(temp);

    if(!packet)
        return NULL;
    return packet;
}

// 큐에 새로운 패킷을 추가
extern void queue_push_packet_new(type_queue **queue, type_packet *packet) {
    type_queue *temp;

    if(!queue)
        return;
    if(!packet)
        return;

    temp = *queue;

    if(!temp) {
        if(!(temp = malloc(sizeof(type_queue))))
            return;
        temp->next = NULL;
        *queue = temp;
    }
    else
    {
        for(;temp->next;temp=temp->next);
        temp->next = malloc(sizeof(type_queue));
        temp->next->next = NULL;
        temp = temp->next;
    }
    temp->packet = packet;
}

extern int queue_get_length(type_queue const * const * queue)
{
    type_queue const *temp;
    int count;

    if(!queue)
        return 0;
    for(temp=*queue, count=0; temp; temp=temp->next, count++);
    return count;
}