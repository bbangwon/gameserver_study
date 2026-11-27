#include <stdlib.h>
#include "objlist.h"

extern int add_node_objlist(type_objlist **objlist, void *data) {
    return 1;
}

extern type_objlist **find_node_objlist(type_objlist **objlist, void *data) {
    return NULL;
}

extern type_objlist *pop_data(type_objlist **objlist) {
    return NULL;
}

extern int insert_node_objlist(type_objlist **target_objlist, type_objlist *source_objlist) {
    return 1;
}

extern int rm_objlist(type_objlist **objlist) {
    return 1;
}

