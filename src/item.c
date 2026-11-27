#include <stdio.h>
#include <sys/time.h>

#include "item.h"
#include "publicInc.h"

static type_itemSect 	item_sect[S_WIDTH][S_WIDTH];
static type_item item0[_MAX_DEFAULT_ITEM]; //일반 아이템

static void *voiditemlist_get_first(type_objlist const * const **save, unsigned short i, unsigned short j) {
    return NULL;
}

static void *voiditemlist_get_next(type_objlist const * const **save) {
    return NULL;
}

extern void item_init() {
    int i;
    for(i = 0; i < _MAX_DEFAULT_ITEM; i++) {
        item0[i].is_init = 0;
    }
}

extern void item_load(type_item *item) {
    item0[item->item_no].is_init = item->is_init;
    item0[item->item_no].idx_type = item->idx_type;
    item0[item->item_no].item_no = item->item_no;
    item0[item->item_no].equip = item->equip;
    item0[item->item_no].item_type = item->item_type;
    item0[item->item_no].req = item->req;
    item0[item->item_no].pwr = item->pwr;
    item0[item->item_no].range = item->range;
    item0[item->item_no].itemclass = item->itemclass;
    item0[item->item_no].amount = item->amount;
    item0[item->item_no].coin = item->coin;
    item0[item->item_no].isbuy = item->isbuy;
}

extern void item_3x3_sectorinfo(type_session *c) {
}

extern void item_part_sectorinfo(type_session *c) {
}

extern unsigned short item_get_attack(unsigned short item_no) {
    return 0;
}

extern unsigned short item_get_defense(unsigned short item_no) {
    return 0;
}

extern unsigned short item_get_range(unsigned short item_no) {
    return 0;
}

extern short item_ret_default_cnt(unsigned short idx){
    return 0;
}

extern unsigned int retNullitemIdx()
{
    return 0;
}

extern void setitemIdx(int idx){

}

extern type_itemLst_field * item_add_sect(unsigned short x, unsigned short y, type_itemLst_field * fitem) {
    return NULL;
}

void *Thread_item_sync(void *arg) {
    int i,j,itm_cnt;
    type_objlist const * const *tmp_objlist;
    void *ntmp_itm;
    type_itemLst_field *tmp_itm;
    struct timeval stop;

    while(1) {
        itm_cnt = 0;
        pthread_mutex_lock(&itemlock);
        for(i=0;i<S_WIDTH;i++) {
            for(j=0;j<S_WIDTH;j++) {
                for(tmp_itm=voiditemlist_get_first(&tmp_objlist,i,j);tmp_itm; tmp_itm=ntmp_itm) {
                    itm_cnt++;
                    ntmp_itm = voiditemlist_get_next(&tmp_objlist);
                    
                    if(tmp_itm->rmFcnt>30) item_rm_sect(tmp_itm);
                    else tmp_itm->rmFcnt++;
                }
            }
        }
        pthread_mutex_unlock(&itemlock);
        stop.tv_sec = 1;
        stop.tv_usec = 0;
        select(0, NULL, NULL, NULL, &stop);
    }
}

extern void item_rm_sect(type_itemLst_field * fitem) {
    //일정 시간이 지나면 아이템을 맵 섹터 리스트에서 삭제
    type_objlist **pos;
    unsigned short Ax, Ay;
    Ax = fitem->x/S_UNITSIZE;
    Ay = fitem->y/S_UNITSIZE;

    if(!fitem) {
        printf("item null\n");
        return;
    }

    resetitemIdx(fitem->idx);
    if(item_sect[Ax][Ay].objL == NULL) {
        printf("itemArray NULL list in item_rm_sect arr:%d,%d 좌표 %d, %d\n", Ax, Ay, fitem->x, fitem->y);
        return;
    }

    pos = find_node_objlist(&(item_sect[Ax][Ay].objL), fitem);

    rm_objlist(pos);

}

extern void resetitemIdx(int idx) {
}
