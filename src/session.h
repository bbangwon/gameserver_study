#pragma once

typedef enum
{
    conn_state_empty,	//not authed
    conn_state_authed,	//lobby status
    conn_state_game,		//status game
    conn_state_destroy	//status exited
} type_conn_state;

typedef struct partySystem {
    unsigned char flag; // 파티 참석 여부
    unsigned char cnt; //파티 인원수

    short memidx; //자신의 위치
    unsigned short infoFcnt; //유저 정보 알림 카운트

    void *waitfor; //기다리고 있는 유저 번호
    void *master;
    short ison1; //0: NULL, 1: linked, 2: myself(member#==NULL)

    void *mem2;
    short ison2;

    void *mem3;
    short ison3;
} type_party;

typedef struct tradeSystem {
    short flag; // 트레이드 중 여부
    void *tg;
    short confirm;

    unsigned char itemidx[2]; //트레이드 인벤토리 실제 저장 값은 인벤토리 인덱스
    unsigned short invidx[2]; //아이템 인덱스
    unsigned short cnt[2]; //아이템 카운트
    unsigned int coin;
} type_trade; //상거래를 위한 구조체

typedef struct connection {
    char userid[32];
    char char_name[32];
    unsigned short uLen;
    unsigned short nLen;
    unsigned short userNo;
    int sock;
    unsigned short port;
    unsigned int addr;
    type_conn_state state;
    unsigned int sessionkey;
    unsigned int clientver;
    int disco;

    unsigned char dbLoaded; //데이터베이스 로드 유무
    short frameCnt; //프레임 카운트
    short userStat; //유저 상태 0: 서있음 1: 이동 2: 전투 3: 거래
    short roomNo; //기본: 0

    /* 캐릭터 관련 */
    unsigned short Cx; //현재위치 x
    unsigned short Cy; //현재위치 y
    unsigned short Dx; //목적지 x
    unsigned short Dy; //목적지 y

    short Ax; //현재 섹터 X
    short Ay; //현재 섹터 Y
    short Bx; //이전 섹터 X
    short By; //이전 섹터 Y

    char race;
    char sex;
    char nation;

    unsigned short str; //힘
    unsigned short dex; //민첩
    unsigned short intel; //지능

    unsigned short attack; //공격력
    unsigned short defense; //방어력

    unsigned short hp_m; //최대 체력
    short hp_c; //현재 체력
    short mana_m; //최대 마나
    short mana_c; //현재 마나
    unsigned int exp; //경험치
    unsigned short level; //레벨
    unsigned short lvpoint; //레벨 포인트
    unsigned int skexp; //스킬 경험치

    char jobno; //직업 번호
    char classno; //클래스 번호
    char classlevel; //클래스 레벨

    unsigned short worldmap; //맵 인덱스
    unsigned short moveLevel; //캐릭터 움직임 레벨
    char Dir; // 목적지 방향(tile no.)

    /* 타겟 */
    char tgtype; //타겟 타입
    short target_id; //타겟 아이디
    void *target; //타겟 구조체

    /* 아이템 정의 */
    unsigned short eq[7]; //장비 배열

    /* 인벤토리 정의 */
    unsigned short inven[4]; //인벤토리 4개
    unsigned char inven_cnt[4]; //인벤토리 각 슬롯의 아이템 개수

    /* 창고 정의 */
    unsigned short ware[8]; //창고 8개
    unsigned char ware_cnt[8]; //창고 각 슬롯의 아이템 개수

    /* 스킬 정의 */
    unsigned short skill[4];
    unsigned char skill_lv[4];

    unsigned int coin; //돈

    char move_F_cnt;
    unsigned short mana_F_cnt; //캐릭터 마나 회복용 카운트
    unsigned short recov_F_cnt; //캐릭터 체력 회복용 카운트
    short attack_F_cnt; //캐릭터 공격 지연용 카운트
    short regen_F_cnt;
    unsigned int dbSaveCnt;

    unsigned char pkmode;
    type_party party;
    type_trade trade;
} type_session;

typedef struct connArray {
    type_session s;
    int sockNo;
    char flag;
}t_sessionArray;

type_session* ret_session_first(unsigned int max);
type_session* ret_session_next(unsigned int max);
type_session* session_create(int sock, unsigned int addr, unsigned short port);
void session_clear(type_session  * c);

void session_set_userid(type_session *c, char *userid, unsigned short uLen);