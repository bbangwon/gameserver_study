#pragma once

 // 맵에서 섹터에 들어오는 유저의 정보를 연결리스트로  만든 자료구조
 typedef struct objlist {
    struct objlist *next;
    void *data;
 } type_objlist;

int add_node_objlist(type_objlist **objlist, void *data);
type_objlist **find_node_objlist(type_objlist **objlist, void *data);
int rm_objlist(type_objlist **objlist);
void *get_node_objlist(type_objlist const *objlist);
type_objlist const * const * get_next_node_objlist_const(type_objlist const *objlist);
type_objlist *pop_data(type_objlist **objlist);
int insert_node_objlist(type_objlist **target_objlist, type_objlist *source_objlist);