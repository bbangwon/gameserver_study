#include <stdlib.h>
#include "objlist.h"

// 객체 리스트에 노드를 추가한다.
extern int add_node_objlist(type_objlist **objlist, void *data) {
    type_objlist *node;
    if(!objlist)
        return -1;

    if(!(node = malloc(sizeof(type_objlist))))
        return -1;

    node->data = data;
    node->next = *objlist;
    *objlist = node;
    return 0;
}

// 객체 리스트와 객체를 매개변수로 받아 객체를 찾는다. 객체 리스트가 연결 리스트이므로 연결 리스트를 검색해야 한다.
extern type_objlist **find_node_objlist(type_objlist **objlist, void *data) {
    type_objlist **tmps;
    if(!objlist)
        return NULL;
    if(!*objlist)
        return NULL;
    tmps = objlist;
    if((*tmps)->data==data)
        return tmps;
    while((*tmps)->next && (*tmps)->next->data != data)
        tmps = &(*tmps)->next;
    if((*tmps)->next)
        return &(*tmps)->next;
    return NULL;
}

// find_node_objlist로 찾은 노드를 삭제하고, 다음 노드로 대체하는 함수. 연결리스트에서 중간 노드를 빼내는 개념이다.
extern int rm_objlist(type_objlist **objlist) {
    type_objlist const *tmps;

    if(!objlist)
        return -1;
    tmps = *objlist;
    if(!tmps)
        return -1;

    *objlist = (*objlist)->next;
    free((void *)tmps);
    return 0;
}

// 객체 리스트에서 노드의 데이터를 리턴한다.
extern void *get_node_objlist(type_objlist const *objlist) {
    if(!objlist)
        return NULL;
    return objlist->data;
}

extern type_objlist const * const * get_next_node_objlist_const(type_objlist const *objlist) {
    if(!objlist)
        return NULL;
    return (type_objlist const * const *)&objlist->next;
}

// 객체리스트에서 해당 노드를 팝업시킨다. 
extern type_objlist *pop_data(type_objlist **objlist) {
    type_objlist *tmps;
    if(!objlist)
        return NULL;
    tmps = *objlist;
    if(!tmps)
        return NULL;
    *objlist = (*objlist)->next;
    return tmps;
}

// 객체 리스트에 노드를 삽입한다.
extern int insert_node_objlist(type_objlist **target_objlist, type_objlist *source_objlist) {
    if(!target_objlist)
        return -1;
    if(!source_objlist)
        return -1;
    source_objlist->next = *target_objlist;
    *target_objlist = source_objlist;
    return 0;
}