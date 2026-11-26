 #pragma once
 
 typedef struct objlist {
    struct objlist *next;
    void *data;
 } type_objlist;

 int add_node_objlist(type_objlist **objlist, void *data);