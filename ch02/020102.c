 typedef enum
 {
    conn_state_empty,
    conn_state_authed,
    conn_state_game,
    conn_state_destroy
 } type_conn_state;

 #define MAX_PACKET_SIZE 0x400

 typedef struct connection {
    char userid[32]; //유저 아이디
    char char_name[32]; //캐릭터 이름
    unsigned short uLen; //유저 아이디 길이
    unsigned short nLen; //캐릭터 이름 길이
    unsigned short userNo; //접속 시 생성되는 캐릭터 아이디 번호
    int sock; //생성된 소켓 번호
    unsigned int addr; //클라이언트 주소
    unsigned int sessionkey; //채팅 서버에서는 사용하지 않음-서버 이동 시 비밀키
    unsigned int clientver; //게임 버전
    int disco;
    unsigned short port; //포트 번호(채팅 서버에서는 따로 사용 안 함)
    short frameCnt; //프레임 카운트
    short userStat; //유저 상태
    short roomNo; // default:0                                                              
    short Ax; //현재 위치 x좌표
    short Ay; //현재 위치 y좌표
    short Bx; //이전 위치 X좌표
    short By; //이전 위치 Y좌표
    type_conn_state state; //클라이언트 상태
 } type_session;

 typedef struct connArray {
    type_session s;
    int sockNo;
    char flat; //0: not use, 1: use

 } t_sessionArray;

 //클라이언트에 보내질 패킷의 형
 typedef struct packet {
    int cli_sock;
    int size;
    char data[MAX_PACKET_SIZE];
    void *session;
 } type_packet;

 //송신할 패킷을 큐 구조로 저장
 typedef struct queues {
    type_packet *packet;
    struct queues *next;
 } type_queues;

 // 맵에서 섹터에 들어오는 유저의 정보를 연결리스트로  만든 자료구조
 typedef struct objlist {
    struct objlist *next;
    void *data;
 } type_objlist;

 // 2차원 배열, 맵의 섹터 구조를 나타냄
 typedef struct mapsys {
    type_objlist *objL;
 } type_map;
