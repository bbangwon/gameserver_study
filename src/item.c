#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "map.h"
#include "item.h"
#include "publicInc.h"

static type_itemSect 	item_sect[S_WIDTH][S_WIDTH];
static type_itemIdx 		itemNo[_MAX_ITEM_IDX];
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

// 필드에 아이템 버리기
extern short item_drop(void * chr,char * msg) {
    type_itemLst_field *temp;
    type_session *c;
    unsigned int ItmNo;
    int res = 0;
    short dLen =2;
    char data[32];
    unsigned char invidx,cnt=0;
    unsigned short nEnd = PEND;

    c=(type_session *)chr;

    invidx = msg[3];
    cnt=msg[4];
    if(invidx>4) res=1;
    else if(c->inven_cnt[invidx] < cnt||cnt==0) res=2;

    if(res==0) {
        if(!(temp = malloc(sizeof(type_itemLst_field)))) return 0;
        temp->rmFcnt = 0;
        temp->rmHcnt = 0;

        pthread_mutex_lock(&itemlock);
        temp->item_idx_cnt=cnt;
        ItmNo = retNullitemIdx();
        setitemIdx(ItmNo);
        temp->idx = ItmNo;
        item_add_sect(c->Ax, c->Ay, temp);
        temp->x=c->Cx;
        temp->y=c->Cy;
        temp->item_idx=c->inven[invidx];
        pthread_mutex_unlock(&itemlock);

        pthread_mutex_lock(&synclock);
        if(cnt==c->inven_cnt[invidx]) {
            c->inven_cnt[invidx] = 0;
            c->inven[invidx] = 0;
        } else {
            c->inven_cnt[invidx] -= cnt;
        }
        pthread_mutex_unlock(&synclock);

        data[dLen] = PK_OBJ_ADD;
        dLen+=1;
        data[dLen] = T_ITEM;
        dLen+=1;
        data[dLen] = 1;
        dLen+=1;
        memcpy(&data[dLen], &temp->idx,2);
        dLen+=2;
        memcpy(&data[dLen], &temp->x, 2);
        dLen+=2;
        memcpy(&data[dLen], &temp->y, 2);
        dLen+=2;
        memcpy(&data[dLen], &temp->item_idx, 2);
        dLen+=2;
        memcpy(&data[dLen], &temp->item_idx_cnt, 2);
        dLen+=2;
        memcpy(&data[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&data[0], &dLen, 2);
        map_usersend_All(c->Ax, c->Ay, data, dLen, c);
    }
    dLen=2;
    data[dLen] = PK_ITEM_DROP;
    dLen+=1;
    data[dLen] = res;
    dLen+=1;
    data[dLen] = invidx;
    dLen+=1;
    memcpy(&data[dLen], &c->inven[invidx], 2);
    dLen+=2;
    data[dLen] = c->inven_cnt[invidx];
    dLen+=1;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c,data,dLen);
    return 1;
}

extern void item_pick(char * data, void * chrr){
    type_session *c;
    char msg[32];
    short Len=2;
    unsigned char i,chk=0,res=0;
    unsigned short idx,tmp_idx, tmp_cnt, tmpx, tmpy;
    unsigned short nEnd = PEND;

    c=(type_session *)chrr;
    memcpy(&idx, &data[3], 2);
    if(idx==0) return;
    if(itemNo[idx].flag == 0) return;

    tmp_idx=itemNo[idx].item->item_idx;
    tmp_cnt=itemNo[idx].item->item_idx_cnt;
    tmpx=itemNo[idx].item->x;
    tmpy=itemNo[idx].item->y;

    //한개 겹침 가능
    if(itemNo[idx].item->item_idx<=_ITEM_DUP_NO_RANGE&&tmp_cnt==1) {
        for(i=0;i<4;i++) {
            if(c->inven[i] == tmp_idx) {
                if(c->inven_cnt[i] < _MAX_ITEM_DUP) {
                    pthread_mutex_lock(&itemlock);
                    itemNo[idx].item->item_idx_cnt-=1;
                    if(itemNo[idx].item->item_idx_cnt == 0) {
                        itemNo[idx].item->item_idx=0;
                        item_rm_sect(itemNo[idx].item);
                        chk=1;
                    } else chk=2;
                    pthread_mutex_unlock(&itemlock);
                    pthread_mutex_lock(&synclock);
                    c->inven_cnt[i]+=1;
                    pthread_mutex_unlock(&synclock);
                    break;
                }
            }
            if(c->inven[i]==0){
                pthread_mutex_lock(&itemlock);
                itemNo[idx].item->item_idx=0;
                itemNo[idx].item->item_idx_cnt=0;
                item_rm_sect(itemNo[idx].item);
                pthread_mutex_unlock(&itemlock);
                pthread_mutex_lock(&synclock);
                c->inven[i]=tmp_idx;
                c->inven_cnt[i]=1;
                pthread_mutex_unlock(&synclock);
                chk=1;
                break;
            }
        }
    } else {
        for(i=0;i<4;i++) {
            if(c->inven[i]==0) {
                pthread_mutex_lock(&itemlock);
                itemNo[idx].item->item_idx=0;
                itemNo[idx].item->item_idx_cnt=0;
                item_rm_sect(itemNo[idx].item);
                pthread_mutex_unlock(&itemlock);
                pthread_mutex_lock(&synclock);
                c->inven[i]=tmp_idx;
                c->inven_cnt[i]=tmp_cnt;
                pthread_mutex_unlock(&synclock);
                chk=1;
                break;
            }
        }
    }

    if(chk==0) res=1;
    msg[Len]=PK_ITEM_PICK;
    Len+=1;
    msg[Len]=res;
    Len+=1;
    memcpy(&msg[Len], &idx, 2);
    Len+=2;
    if(res==0){
        msg[Len]=i;
        Len+=1;
        msg[Len]=c->inven_cnt[i];
        Len+=1;
    }
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
    if(chk == 1) {
        item_remove_info(tmpx, tmpy, idx);
    }

}

//필드 아이템 삭제 정보
extern void item_remove_info(unsigned short x,unsigned short y,unsigned short idx){
    char data[16];
    short dLen = 2;
    unsigned short nEnd = PEND;
    unsigned short tmpx, tmpy;

    data[dLen]=PK_OBJ_REMOVE;
    dLen+=1;
    data[dLen]=T_ITEM;
    dLen+=1;
    memcpy(&data[dLen], &idx, 2);
    dLen+=2;
    memcpy(&data[dLen], &x, 2);
    dLen+=2;
    memcpy(&data[dLen], &y, 2);
    dLen+=2;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);

    tmpx = x/S_UNITSIZE;
    tmpy = y/S_UNITSIZE;

    map_usersend_All(tmpx, tmpy, data, dLen, NULL);
}