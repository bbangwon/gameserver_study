#pragma once
#include "session.h"

typedef struct item {
    char is_init;
    char idx_type;  //인덱스 타입
    unsigned short item_no; //아이템 번호는 아이템 배열의 인덱스(아이템 고유 번호)
    char equip; //장착위치 0: 머리, 1: 한손, 2: 양손, 3: 몸, 4: 발, 5:목걸이, 6: 반지
    char item_type; //장비 타입
    short req;  //요구 레벨   
    unsigned short pwr; //공격력 또는 방어력
    char range; //타격거리
    char itemclass; //사용계열 0: 무기 1: 갑옷 2: 기타
    unsigned short amount; //기본 개수
    unsigned short coin; //가격
    char isbuy; //구입 가능 여부
} type_item;

void item_init();
void item_load(type_item *item);
void item_3x3_sectorinfo(type_session *c);