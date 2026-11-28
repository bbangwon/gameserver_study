#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "npc.h"
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

extern short item_pos(void * chr,char * msg){
    type_session *c;
    int res = 0;
    short dLen = 2;
    char data[32];
    unsigned char Tidx, Tcnt = 0, Didx, Dcnt = 0;
    unsigned short nEnd = PEND;

    c=(type_session *)chr;

    Tidx=msg[3];
    Tcnt=msg[4];
    Didx=msg[5];
    Dcnt=msg[6];
    Dcnt+=Tcnt;

    if(Tidx>4||Didx>4) res=1;
    else if(c->inven_cnt[Tidx] < Tcnt||Tcnt==0) res=2;
    else if(c->inven[Didx] != 0){ //목적지에 이미 있을때
        if(c->inven[Didx] != 0) res=3;
        else if(c->inven[Didx] > _ITEM_DUP_NO_RANGE) res=4;
        else if(Dcnt > _MAX_ITEM_DUP) res=5;           
    }

    if(res==0){
        pthread_mutex_lock(&synclock);
        if(c->inven_cnt[Tidx] == Tcnt) {
            c->inven[Didx]=c->inven[Tidx];
            c->inven_cnt[Didx]=Dcnt;
            c->inven[Tidx]=0;
            c->inven_cnt[Tidx]=0;
        } else {
            c->inven_cnt[Didx]=Dcnt;
            c->inven_cnt[Tidx]-=Tcnt;
        }
        pthread_mutex_unlock(&synclock);
    }
    dLen=2;
    data[dLen]=PK_ITEM_POS;
    dLen+=1;
    data[dLen]=res;
    dLen+=1;
    data[dLen]=Tidx;
    dLen+=1;
    memcpy(&data[dLen], &c->inven[Tidx], 2);
    dLen+=2;
    data[dLen] = c->inven_cnt[Tidx];
    dLen+=1;
    data[dLen]=Didx;
    dLen+=1;
    memcpy(&data[dLen], &c->inven[Didx], 2);
    dLen+=2;
    data[dLen]=c->inven_cnt[Didx];
    dLen+=1;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);
    return 1;
}

extern short item_wear(void * chr,char * msg){
    type_session *c;
    int res=0, tmp;
    short dLen = 2;
    char data[32];
    unsigned char invidx, equip;
    unsigned short nEnd = PEND;

    c=(type_session *)chr;

    invidx = msg[3];
    equip = msg[4];

    if(invidx>4||equip>6) res=1;
    else if(c->inven[invidx] == 0 && c->eq[equip] == 0) res=2; 
    else if(c->inven[invidx] != 0 && c->eq[equip] != 0) {
        tmp = c->inven[invidx];
        if(item0[tmp].equip==equip) {
            pthread_mutex_lock(&synclock);
            c->inven[invidx]=c->eq[equip];
            c->eq[equip]=tmp;
            item_recal_stat(c);
            pthread_mutex_unlock(&synclock);
        } else res =3;
    }else {
        pthread_mutex_lock(&synclock);
        if(c->inven[invidx] == 0) { //벗을때
            c->inven[invidx]=c->eq[equip];
            c->inven_cnt[invidx]=1;
            c->eq[equip]=0;
            item_recal_stat(c);
        }else { //입을때
            tmp=c->inven[invidx];
            if(item0[tmp].equip==equip) {
                c->eq[equip]=c->inven[invidx];
                c->inven_cnt[invidx]=0;
                c->inven[invidx]=0;
                item_recal_stat(c);
            }else res=3;
        }
        pthread_mutex_unlock(&synclock);
    }
    data[dLen]=PK_ITEM_WEAR;
    dLen+=1;
    data[dLen]=res;
    dLen+=1;
    data[dLen]=invidx;
    dLen+=1;
    memcpy(&data[dLen], &c->inven[invidx], 2);
    dLen+=2;
    data[dLen] = equip;
    dLen+=1;
    memcpy(&data[dLen], &c->eq[equip], 2);
    dLen+=2;

    memcpy(&data[dLen], &c->attack, 2);
    dLen+=2;
    memcpy(&data[dLen], &c->defense, 2);
    dLen+=2;
    memcpy(&data[dLen], &c->hp_m, 2);
    dLen+=2;
    memcpy(&data[dLen], &c->mana_m, 2);
    dLen+=2;

    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);

    if(res==0) {
        dLen=2;
        data[dLen]=PK_OBJ_UPDATE_EQUI;
        dLen+=1;
        memcpy(&data[dLen], &c->userNo,2);
        dLen+=2;
        data[dLen]=equip;
        dLen+=1;
        memcpy(&data[dLen], &c->eq[equip], 2);
        dLen+=2;

        memcpy(&data[dLen], &c->attack, 2);
        dLen+=2;
        memcpy(&data[dLen], &c->defense, 2);
        dLen+=2;
        memcpy(&data[dLen], &c->hp_m, 2);
        dLen+=2;
        memcpy(&data[dLen], &c->mana_m, 2);
        dLen+=2;

        memcpy(&data[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&data[0], &dLen, 2);
        map_usersend_All(c->Ax, c->Ay, data, dLen, c);
    }
    return 1;
}

//창고 아이템 꺼내기/저장하기
extern void item_ware_depos(void * chr,char * dat){
    type_session *c;
    char msg[128];
    short Len=2;
    int res=0;
    unsigned short nEnd=PEND, tmp, tmpcnt;
    unsigned char invidx=0, wareidx=0;

    c = (type_session *)chr;
    if(c == NULL) return;

    invidx = dat[3];
    wareidx = dat[4];

    if(c->inven[invidx] == 0 && c->ware[wareidx] == 0) res=1;
    else {
        pthread_mutex_lock(&synclock);
        tmp=c->ware[wareidx];
        tmpcnt=c->ware_cnt[wareidx];
        c->ware[wareidx] = c->inven[invidx];
        c->ware_cnt[wareidx] = c->inven_cnt[invidx];
        c->inven[invidx]=tmp;
        c->inven_cnt[invidx] = tmpcnt;
        pthread_mutex_unlock(&synclock);
    }

    msg[Len]=PK_WARE_DEPOS;
    Len+=1;
    msg[Len]=res;
    Len+=1;
    msg[Len]=invidx;
    Len+=1;
    memcpy(&msg[Len], &c->inven[invidx], 2);
    Len+=2;
    msg[Len]=c->inven_cnt[invidx];
    Len+=1;
    msg[Len]=wareidx;
    Len+=1;
    memcpy(&msg[Len], &c->ware[wareidx], 2);
    Len+=2;
    msg[Len]=c->ware_cnt[wareidx];
    Len+=1;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

//창고 아이템 위치 바꾸기
extern void item_ware_pos(void * chr,char * dat){
    type_session *c;
    char msg[128];
    short Len=2;
    int res=0;
    unsigned short nEnd=PEND, tmp, tmpcnt;
    unsigned char tgidx=0, tgcnt=0, desidx=0;

    c = (type_session *)chr;
    if(c == NULL) return;

    tgidx = dat[3];
    tgcnt = dat[4];
    desidx = dat[5];
    if(c->ware[tgidx] == 0) res=1;
    else if(c->ware_cnt[tgidx] < tgcnt) res=2;
    else if(c->ware_cnt[tgidx] != tgcnt){ //부분적으로 옮김
        if(c->ware[desidx] > 0){ //목적지에 아이템이 있을 경우
            if(c->ware[desidx] < _ITEM_DUP_NO_RANGE 
                && c->ware[desidx] == c->ware[tgidx] 
                && _MAX_ITEM_DUP>=tgcnt+c->ware_cnt[desidx]){
                    pthread_mutex_lock(&synclock);
                    c->ware_cnt[desidx] += tgcnt;
                    c->ware_cnt[tgidx] -= tgcnt;
                    pthread_mutex_unlock(&synclock);
            } else res=3;
        }else { //목적지에 아이템이 없는 경우
            pthread_mutex_lock(&synclock);
            c->ware_cnt[desidx] = tgcnt;
            c->ware[desidx] = c->ware[tgidx];
            c->ware_cnt[tgidx] -= tgcnt;
            pthread_mutex_unlock(&synclock);

        } 
    }else { //옮길 수량이 모두일 경우
        if(c->ware[desidx] > 0) {
            if(c->ware[desidx] < _ITEM_DUP_NO_RANGE &&
                c->ware[desidx] == c->ware[tgidx] &&
                _MAX_ITEM_DUP>=tgcnt+c->ware_cnt[desidx]){
                pthread_mutex_lock(&synclock);
                c->ware_cnt[desidx] += tgcnt;
                c->ware[tgidx] = 0;
                c->ware_cnt[tgidx] = 0;
                pthread_mutex_unlock(&synclock);
            } else {
                pthread_mutex_lock(&synclock);
                tmp=c->ware[desidx];
                tmpcnt=c->ware_cnt[desidx];
                c->ware[desidx] = c->ware[tgidx];
                c->ware_cnt[desidx] = c->ware_cnt[tgidx];
                c->ware[tgidx] = tmp;
                c->ware_cnt[tgidx] = tmpcnt;
                pthread_mutex_unlock(&synclock);
            }
        } else {
            pthread_mutex_lock(&synclock);
            c->ware_cnt[desidx] = tgcnt;
            c->ware[desidx] = c->ware[tgidx];
            c->ware[tgidx] = 0;
            c->ware_cnt[tgidx] = 0;
            pthread_mutex_unlock(&synclock);
        }
    }

    msg[Len] = PK_WARE_POS;
    Len+=1;
    msg[Len]=res;
    Len+=1;
    msg[Len]=tgidx;
    Len+=1;
    memcpy(&msg[Len], &c->ware[tgidx], 2);
    Len+=2;
    msg[Len]=c->ware_cnt[tgidx];
    Len+=1;
    msg[Len]=desidx;
    Len+=1;
    memcpy(&msg[Len], &c->ware[desidx], 2);
    Len+=2;
    msg[Len]=c->ware_cnt[desidx];
    Len+=1;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

extern void item_recal_stat(void * chr){

}

extern void item_buy(char * data,void * se){
    unsigned char amount;
    unsigned short itemNo, tmp, nLen;
    int res=0, i, ckk;
    type_session *c;
    short Len=2;
    char msg[32];
    unsigned short nEnd=PEND;

    c=(type_session *)se;
    nLen=npc_dist_chk(c); //상점 NPC와의 거리 체크
    memcpy(&itemNo, &data[3], 2);
    amount=data[5];

    if(amount==0||itemNo==0||itemNo>=_MAX_DEFAULT_ITEM||nLen>15){
        res = 1;
    } else if(amount>_MAX_ITEM_DUP) {
        res = 2;
    } else if(amount>1) {
        if(itemNo>_ITEM_DUP_NO_RANGE) res=3;
    }
    if(res==0 && item0[itemNo].isbuy==0) res=4;
    tmp = item0[itemNo].coin * amount;
    if(res==0 && c->coin < tmp) res=5;

    for(i=0,ckk=0;i<4;i++){
        if(c->inven[i]==0){
            ckk=1;
            break;
        }
    }
    if(res==0 && ckk==0) res=6;
    if(res==0){
        pthread_mutex_lock(&synclock);
        c->inven[i] = itemNo;
        c->inven_cnt[i] = amount;
        c->coin -= tmp;
        pthread_mutex_unlock(&synclock);
    }
    msg[Len] = PK_ITEM_BUY;
    Len+=1;
    msg[Len] = res;
    Len+=1;
    memcpy(&msg[Len],&itemNo, 2);
    Len+=2;
    msg[Len] =amount;
    Len+=1;
    msg[Len]=i;
    Len+=1;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

extern void item_sell(char * data,void * se){
    unsigned char amount, tot, idx=0;
    unsigned short itemNo;
    int res=0, i, j;
    type_session *c;
    short Len=2;
    char msg[32];
    unsigned short nEnd=PEND;

    c=(type_session *)se;

    tot=data[5];
    for(i=0,j=4;i<tot;i++){
        idx=data[j];
        if(idx>4) {
            res=1;
            break;
        }
        j+=1;
        amount=data[j];
        j+=1;
        if(c->inven[idx]>=_MAX_DEFAULT_ITEM||c->inven[idx]==0||
        c->inven_cnt[idx]<amount||amount==0){
            res=2;
            break;            
        }        
    }
    msg[Len] = PK_ITEM_SELL;
    Len+=1;
    msg[Len] = res;
    Len+=1;
    msg[Len] = tot;
    Len+=1;

    if(res==0) {
        pthread_mutex_lock(&synclock);
        for(i=0,j=4;i<tot;i++){
            idx=data[j];
            j+=1;
            amount=data[j];
            j+=1;
            itemNo=c->inven[idx];
            c->coin += item0[itemNo].coin * amount;
            c->inven_cnt[idx] -= amount;
            if(c->inven_cnt[idx]==0) c->inven[idx]=0;

            msg[Len]=idx;
            Len+=1;
            msg[Len]=c->inven_cnt[idx];
            Len+=1;            
        }
        pthread_mutex_unlock(&synclock);
    }
    memcpy(&msg[Len], &c->coin, 4);
    Len+=4;
    memcpy(&msg[Len], &nEnd, 2);
    Len+=2;
    memcpy(&msg[0], &Len, 2);
    map_pData_snd(c, msg, Len);
}

extern short item_trade_req(void * chr,char * msg){
    type_session *c;
    type_session *tc;

    int res=0,i;
    short dLen=2, dist=0;
    char data[32];
    unsigned short nEnd=PEND;
    unsigned short userNo;

    c=(type_session *)chr;
    memcpy(&userNo, &msg[4], 2);
    tc=session_ret_session(userNo);
    if(tc==NULL) res=1;
    else {
        dist=map_ret_dist(c->Cx, c->Cy, tc->Cx, tc->Cy);
        if(dist>15) res=2;
    }
    if(msg[3] != 0) res=3;
    if(res==0) {
        if(c->trade.flag == 0) {
            pthread_mutex_lock(&synclock);
            c->trade.flag =1;
            c->trade.confirm=0;
            c->trade.tg=tc;
            c->trade.coin=0;
            for(i=0;i<2;i++) {
                c->trade.itemidx[i]=0;                
            }
            pthread_mutex_unlock(&synclock);
        } else res=4;
    }

    data[dLen] = PK_TRADE_REQ;
    dLen+=1;
    data[dLen] = msg[3];
    dLen+=1;
    data[dLen] = res;
    dLen+=1;
    memcpy(&data[dLen], &userNo, 2);
    dLen+=2;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);

    if(res==0) {
        dLen=2;
        data[dLen]=PK_TRADE_REQ;
        dLen+=1;
        data[dLen] =1;
        dLen+=1;
        memcpy(&data[dLen], &c->userNo, 2);
        dLen+=2;
        memcpy(&data[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&data[0], &dLen, 2);
        map_pData_snd(tc, data, dLen);
    }
    return 1;
}

extern short item_trade_req_ok(void * chr, char * msg){
    type_session *c;
    type_session *tc;
    int res=0, i;
    short dLen=2, dist=0;
    char data[32];
    unsigned short nEnd=PEND;
    unsigned short userNo;

    c=(type_session *)chr;
    memcpy(&userNo, &msg[4], 2);
    tc=session_ret_session(userNo);
    if(tc==NULL) res=1;
    else {
        dist=map_ret_dist(c->Cx, c->Cy, tc->Cx, tc->Cy);
        if(dist>15) res=2;
    }
    if(res==0&&msg[3]==0){
        if(c->trade.flag==0&&c==tc->trade.tg) {
            pthread_mutex_lock(&synclock);
            c->trade.flag=1;
            c->trade.confirm=0;
            c->trade.tg=tc;
            c->trade.coin=-0;
            for(i=0;i<2;i++) {
                c->trade.itemidx[i]=0;
            }
            pthread_mutex_unlock(&synclock);
        } else res=4;
    }

    data[dLen] = PK_TRADE_REQ_OK;
    dLen+=1;
    data[dLen]=msg[3];
    dLen+=1;
    memcpy(&data[dLen], &c->userNo, 2);
    dLen+=2;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);
    return 1;
}

extern short item_trade_list(void * chr, char * msg){
    type_session *c;
    int res=0, i, tmp;
    short dLen=2;
    char data[32];
    unsigned char cnt=0;
    unsigned short nEnd=PEND;

    c=(type_session *)chr;
    memcpy(&c->trade.coin, &msg[4], 2);
    cnt=msg[4];

    if(c->trade.flag!=1)  res=1;
    else if(cnt==0||cnt>2) res=2;
    pthread_mutex_lock(&synclock);
    if(c->trade.confirm==1) res=3;
    
    for(i=0;i<cnt;i++) {
        c->trade.invidx[i]=msg[i*2+5];
        tmp=c->trade.invidx[i];
        if(c->inven[tmp]==0){
            res=4;
            break;
        }
        c->trade.itemidx[i]=c->inven[tmp];
        c->trade.cnt[i]=msg[i*2+6];
        if(c->trade.cnt[i]>c->inven_cnt[tmp]){
            res=5;
            break;
        }
    }
    pthread_mutex_unlock(&synclock);

    data[dLen] = PK_TRADE_LIST;
    dLen+=1;
    data[dLen] = res;
    dLen+=1;
    memcpy(&data[dLen], &c->userNo, 2);
    dLen+=2;
    memcpy(&data[dLen], &c->trade.coin, 4);
    dLen+=4;
    data[dLen]=cnt;
    dLen+=1;
    for(i=0;i<cnt;i++){
        data[dLen]=c->trade.cnt[i];
        dLen+=1;
        data[dLen]=c->trade.invidx[i];
        dLen+=1;
        memcpy(&data[dLen], &c->trade.itemidx, 2);
        dLen+=2;
    }
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);

    if(res==0) map_pData_snd(c->trade.tg, data, dLen); //성공이면 타겟유저에게도 전송
    return 1;
}

extern short item_trade_ok(void * chr, char * msg){
    type_session *c;
    type_session *tc;
    int res=0,i,j,chk=0,myCnt=0,tgCnt=0,myTcnt=0,tgTcnt=0;
    short dLen=2;
    char data[32];
    unsigned short mytmp[2], tgtmp[2];
    unsigned char mytmpcnt[2], tgtmpcnt[2], mychange[4], tgchange[4];
    unsigned char cnt=0, tmp=0;
    unsigned short nEnd=PEND;

    c=(type_session *)chr;
    tc=(type_session *)c->trade.tg;
    if(c->trade.flag!=1) res=1;
    else if(tc==NULL) res=2;
    else if(tc->trade.flag!=1) res=3;
    if(c->coin<c->trade.coin) res=4;
    if(tc->coin<tc->trade.coin) res=5;

    if(res==0) {
        if(tc->trade.confirm==1) { //상대가 이미 확인했을 때 진행
            pthread_mutex_lock(&synclock);
            for(i=0;i<2;i++) {
                if(c->trade.itemidx[i] != 0){
                    tmp =c->trade.invidx[i];
                    myTcnt+=1;
                    if(tmp>4||c->inven_cnt[tmp]==0){
                        chk=1;
                        break;
                    }
                    if(c->inven[tmp]!=c->trade.itemidx[i]){
                        chk=2;
                        break;
                    }
                    if(c->inven_cnt[tmp]<c->trade.cnt[i]){
                        chk=3;
                        break;
                    } else if(c->inven_cnt[tmp]==c->trade.cnt[i]){
                        myCnt+=1;
                    }
                }
            }
            if(chk==0) {
                for(i=0;i<4;i++) {
                    mychange[i]=0;
                    tgchange[i]=0;
                    if(c->inven[tmp]==0) myCnt+=1;
                }
                for(i=0;i<2;i++) {
                    if(tc->trade.itemidx[i] != 0){
                        tmp=tc->trade.invidx[i];
                        tgTcnt+=1;
                        if(tmp>4||tc->inven_cnt[tmp]==0){
                            chk=4;
                            break;
                        }
                        if(tc->inven[tmp]!=tc->trade.itemidx[i]){
                            chk=5;
                            break;
                        }
                        if(tc->inven_cnt[tmp]<tc->trade.cnt[i]){
                            chk=6;
                            break;
                        } else if(tc->inven_cnt[tmp]==tc->trade.cnt[i]){
                            tgCnt+=1;
                        }
                    }
                }
                if(chk==0) {
                    for(i=0;i<4;i++) {
                        if(tc->inven[tmp]==0) tgCnt+=1;
                    }
                }
            }
            if(chk==0) {
                if(myTcnt>tgCnt) chk=7;
                else if(tgTcnt>myCnt) chk=8;
                else {
                    c->coin += tc->trade.coin;
                    tc->coin -= tc->trade.coin;
                    tc->coin += c->trade.coin;
                    c->coin -= c->trade.coin;

                    for(i=0;i<2;i++) {
                        mytmp[i]=0;
                        if(c->trade.itemidx[i] != 0){
                            tmp =  c->trade.invidx[i];
                            mytmp[i] = c->inven[tmp];
                            mytmpcnt[i] = c->trade.cnt[i];

                            if(c->inven_cnt[tmp] == c->trade.cnt[i]){
                                c->inven[tmp] = 0;
                                c->inven_cnt[tmp] = 0;
                            } else c->inven_cnt[tmp] -= c->trade.cnt[i];
                            mychange[tmp]=1;
                        }
                    }
                    for(i=0;i<2;i++) {
                        tgtmp[i]=0;
                        if(tc->trade.itemidx[i] != 0){
                            tmp = tc->trade.invidx[i];
                            tgtmp[i] = tc->inven[tmp];
                            tgtmpcnt[i] = tc->trade.cnt[i];

                            if(tc->inven_cnt[tmp] == tc->trade.cnt[i]){
                                tc->inven[tmp] = 0;
                                tc->inven_cnt[tmp] = 0;
                            } else tc->inven_cnt[tmp] -= tc->trade.cnt[i];
                            tgchange[tmp]=1;
                        }
                    }
                    for(i=0;i<4;i++) {
                        if(cnt<tgTcnt && c->inven[i] == 0) {
                            mychange[i] = 1;
                            for(j=0;j<2;j++) {
                                if(tgtmp[j] != 0) break;
                            }
                            c->inven[i]=tgtmp[j];
                            c->inven_cnt[i]=tgtmpcnt[j];
                            tgtmp[j] = 0;
                            tgtmpcnt[j] = 0;
                            cnt+=1;
                        }
                    }
                    cnt = 0;
                    for(i=0;i<4;i++) {
                        if(cnt<myTcnt && tc->inven[i] == 0) {
                            tgchange[i] = 1;
                            for(j=0;j<2;j++) {
                                if(mytmp[j] != 0) break;
                            }
                            tc->inven[i]=mytmp[j];
                            tc->inven_cnt[i]=mytmpcnt[j];
                            mytmp[j] = 0;
                            mytmpcnt[j] = 0;
                            cnt+=1;
                        }
                    }
                }
            }
            c->trade.flag = 0;
            tc->trade.flag = 0;

            pthread_mutex_unlock(&synclock);

            dLen=2;
            data[dLen]=PK_TRADE_SUCC;
            dLen+=1;
            memcpy(&data[dLen], &c->coin, 4);
            dLen+=4;
            dLen+=1;
            cnt=0;
            for(i=0;i<4;i++) {
                if(mychange[i]==1) {
                    cnt+=1;
                    data[dLen]=i;
                    dLen+=1;
                    memcpy(&data[dLen], &c->inven[i], 2);
                    dLen+=2;
                    data[dLen]=c->inven_cnt[i];
                    dLen+=1;
                }
            }
            data[7]=cnt;
            memcpy(&data[dLen], &nEnd,2);
            dLen+=2;
            memcpy(&data[0], &dLen, 2);
            map_pData_snd(c, data, dLen);

            dLen=2;
            data[dLen]=PK_TRADE_SUCC;
            dLen+=1;
            memcpy(&data[dLen], &tc->coin, 4);
            dLen+=4;
            dLen+=1;
            cnt=0;
            for(i=0;i<4;i++) {
                if(tgchange[i]==1) {
                    cnt+=1;
                    data[dLen]=i;
                    dLen+=1;
                    memcpy(&data[dLen], &tc->inven[i], 2);
                    dLen+=2;
                    data[dLen]=tc->inven_cnt[i];
                    dLen+=1;
                }
            }
            data[7]=cnt;
            memcpy(&data[dLen], &nEnd,2);
            dLen+=2;
            memcpy(&data[0], &dLen, 2);
            map_pData_snd(tc, data, dLen);
        } else {
            pthread_mutex_lock(&synclock);
            c->trade.confirm=1;
            pthread_mutex_unlock(&synclock);
            dLen=2;
            data[dLen]=PK_TRADE_OK;
            dLen+=1;
            data[dLen]=res;
            dLen+=1;
            memcpy(&data[dLen], &c->userNo, 2);
            dLen+=2;
            memcpy(&data[dLen], &nEnd, 2);
            dLen+=2;
            memcpy(&data[0], &dLen, 2);
            map_pData_snd(c, data, dLen);
            map_pData_snd(tc, data, dLen);
        }
    }
    if(res>0) {
        dLen=2;
        data[dLen]=PK_TRADE_OK;
        dLen+=1;
        data[dLen]=res;
        dLen+=1;
        memcpy(&data[dLen], &c->userNo, 2);
        dLen+=2;
        memcpy(&data[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&data[0], &dLen, 2);
        map_pData_snd(c, data, dLen);        
    }
    return 1;
}

extern short item_trade_cancel(void * chr, char * msg){
    type_session *c;
    type_session *tc;
    int res=0;
    short dLen=2;
    char data[32];
    unsigned short nEnd=PEND;

    c=(type_session *)chr;
    tc=(type_session *)c->trade.tg;
    if(tc==NULL) res=1;
    else if(tc->trade.tg!=c)res=1;

    if(res==0){
        pthread_mutex_lock(&synclock);
        c->trade.flag=0;
        tc->trade.flag=0;
        pthread_mutex_unlock(&synclock);
        data[dLen]=PK_TRADE_CANCEL;
        dLen+=1;
        data[dLen]=0;
        dLen+=1;
        memcpy(&data[dLen], &c->userNo, 2);
        dLen+=2;
        memcpy(&data[dLen], &nEnd, 2);
        dLen+=2;
        memcpy(&data[0], &dLen, 2);
        map_pData_snd(c, data, dLen);
        map_pData_snd(tc, data, dLen);
        return 1;
    }

    data[dLen]=PK_TRADE_CANCEL;
    dLen+=1;
    data[dLen]=res;
    dLen+=1;
    memcpy(&data[dLen], &c->userNo, 2);
    dLen+=2;
    memcpy(&data[dLen], &nEnd, 2);
    dLen+=2;
    memcpy(&data[0], &dLen, 2);
    map_pData_snd(c, data, dLen);
    return 1;
}

