 #pragma once
 
 typedef struct objlist {
    struct objlist *next;
    void *data;
 } type_objlist;

 int add_node_objlist(type_objlist **objlist, void *data);
 type_objlist **find_node_objlist(type_objlist **objlist, void *data);
 int rm_objlist(type_objlist **objlist);
 type_objlist *pop_data(type_objlist **objlist);
 int insert_node_objlist(type_objlist **target_objlist, type_objlist *source_objlist);
