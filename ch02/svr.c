//기본 스레드와 소켓 I/O를 다루는 함수
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <libpq-fe.h>
#include <errno.h>

#include "session.h"
#include "packet.h"
#include "thr_act.h"
#include "que.h"
#include "svr.h"

#define _G_TCP_PORT 9901
#define _G_UDP_PORT 8801

#define LISTEN_QUEUE 0x0A

typedef struct sockPair {
    int tcpSock;
    int udpSock;
} type_sockpair;

void * Thread_recvpack(void *arg);
void * Thread_sendpack(void *arg);
void * Thread_udpserv(void *arg);

void setup_signals();

int tUsercnt = 0;

type_queue  * send_que=NULL;

int signal_sock_no	= 0;

pthread_mutex_t synclock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	synccond	= PTHREAD_COND_INITIALIZER;
pthread_mutex_t sendQuelock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	sendQuecond	= PTHREAD_COND_INITIALIZER;

int main() {
    int tcpsock, udpsock, tresult;
    struct sockaddr_in laddr, udservaddr;

    /* 소켓 옵션 설정을 위한 linger 구조체 
    // linger 구조체는 소켓이 닫힐 때 남아있는 데이터를 처리하는 옵션을 설정하는데 사용됨
    // l_onoff: linger 옵션 사용 여부 (1: 사용, 0: 사용 안함)
    // l_linger: 소켓이 닫힐 때 남아있는 데이터를 처리하는 시간 (초)    
    */
    struct linger ling = {1, 0}; 

    type_sockpair sockPair;
    pthread_t thread_id[4];
    pthread_attr_t attr;
    const int val=1;

    if((tcpsock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("error in sock\n");
        return -1;
    }

    if ((udpsock = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("error in udpsock\n");
        return -1;
    }

    if(setsockopt(tcpsock, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int))<0)
        printf("error in setsockopt\n");

    if (setsockopt(tcpsock, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(struct linger)) < 0)
        printf("[initialize_communication()] can't setsockopt (SO_LINGER)\n");

    memset(&laddr, 0, sizeof(laddr));
    laddr.sin_family = AF_INET;
    laddr.sin_port = htons((unsigned short) _G_TCP_PORT);
    laddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bzero(&udservaddr, sizeof(udservaddr)); // UDP 서버 주소 구조체 초기화 - bzero : 메모리를 0으로 설정하여 초기화
    udservaddr.sin_family = AF_INET;
    udservaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    udservaddr.sin_port = htons((unsigned short) _G_UDP_PORT);

    if(bind(udpsock, (struct sockaddr*)&udservaddr, sizeof(udservaddr)) < 0){
        printf("error in bind\n");        
        return -1;
    }

    if(bind(tcpsock, (struct sockaddr*)&laddr, sizeof(laddr)) < 0) {
        printf("error in bind\n");
        close(udpsock);
        return -1;
    }

    if(fcntl(tcpsock, F_SETFL, O_NONBLOCK) == -1) {
        printf("error in fcntl\n");
        close(tcpsock);
        close(udpsock);
        return -1;
    }

    if (listen(tcpsock, LISTEN_QUEUE) < 0)
    {
        printf("error in listen\n");
        close(tcpsock);
        close(udpsock);
        return -1;
    }
    sockPair.tcpSock = tcpsock;
    sockPair.udpSock = udpsock;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

    init_t_sessionArray();
    setup_signals();
    
    tresult = pthread_create(&thread_id[0], &attr, Thread_sendpack, NULL);
    if(tresult != 0) printf("thread creation failed %d\n", tresult);
    tresult = pthread_create(&thread_id[1], &attr, Thread_sync, NULL);
    if(tresult != 0) printf("thread creation failed %d\n", tresult);
    tresult = pthread_create(&thread_id[2], &attr, Thread_recvpack, &sockPair);
    if(tresult != 0) printf("thread creation failed %d\n", tresult);
    tresult = pthread_create(&thread_id[3], &attr, Thread_udpserv, &udpsock);
    if(tresult != 0) printf("thread creation failed %d\n", tresult);

    for(;;)
        pause();
    
    close(tcpsock);
    close(udpsock);

    return 0;    
}

void * Thread_recvpack(void *arg)
{
    int tcpsock;
    unsigned char msg[MAX_PACKET_SIZE], msgto[32][MAX_PACKET_SIZE];
    int highest_fd;
    int csocket, pCnt, mx;
    int n;
    short rPsize[8];
    type_sockpair sockPair;
    unsigned short rEnd;
    unsigned short nEnd = PEND;
    fd_set rfds, efds;
    int divIdx, sizee;

    int state;
    PGconn *conn;  
    
    type_session *c;
    type_session *nc;

    sockPair = *((type_sockpair *)arg);
    tcpsock = sockPair.tcpSock;

    conn = PQconnectdb("host=localhost port=5432 dbname=mydb user=postgres password=postgres");
    if (PQstatus(conn) != CONNECTION_OK) {
        printf("postgres connection failed: %s\n", PQerrorMessage(conn));
        return NULL;
    }

    for(;;) {
        FD_ZERO(&rfds); // FD_ZERO : fdset 초기화
        FD_ZERO(&efds);
        highest_fd = tcpsock;
        FD_SET(tcpsock, &rfds); // FD_SET : fdset 중 소켓 fd에 해당하는 비트 설정
        for(nc=ret_session_first(tUsercnt); nc; nc=ret_session_next(tUsercnt))
        {
            csocket = nc->sock;
            FD_SET(csocket, &rfds);
            FD_SET(csocket, &efds);

            if(csocket > highest_fd)
                highest_fd = csocket;
        }

        // select 호출 전에 준비 완료된 소켓을 확인하기 위해 주석 추가
        if (select(highest_fd+1, &rfds, NULL, &efds, (struct timeval *)0) < 0) {
            if(errno != EINTR)
                continue;
        }

        // 접속 요청
        if(FD_ISSET(tcpsock, &rfds)) { //FD_ISSET : fdset 중 소켓 fd에 해당하는 비트가 설정되어 있으면 양수 값 리턴
            int asock;
            struct sockaddr_in caddr;
            unsigned int caddr_len;

            //접속 허용
            caddr_len = sizeof(caddr);
            if((asock = accept(tcpsock, (struct sockaddr *)&caddr, &caddr_len)) < 0){
                if(errno == EINTR)
                    continue;
                else
                    printf("error with accept\n");
                continue;
            }else {

            }
            
            // 소켓을 논블로킹 모드로 설정
            if(fcntl(asock, F_SETFL, O_NONBLOCK) < 0)
            {
                printf("error with fcntl\n");
                shutdown(asock, 2);
                close(asock);
                continue;
            }

            // 세션 생성 및 등록
            pthread_mutex_lock(&synclock);
            if(!(c = session_create(asock, ntohl(caddr.sin_addr.s_addr), ntohs(caddr.sin_port))))
            {
                printf("error with session_create\n");
                pthread_mutex_unlock(&synclock); //중요
                shutdown(asock, 2);
                close(asock);
                continue;
            }
            pthread_mutex_unlock(&synclock);
            tUsercnt++;
        }

        // 세션 상태 확인 및 정리
        for(c=ret_session_first(tUsercnt); c; c=ret_session_next(tUsercnt))
        {
            if(c->state == conn_state_destroy)
            {
                tUsercnt--;
                printf("aborted connection\n");
                pthread_mutex_lock(&synclock);
                session_clear(c);
                pthread_mutex_unlock(&synclock);
                continue;
            }

            csocket = c->sock;
            if(FD_ISSET(csocket, &rfds)) {
                memset(msg, 0, sizeof(msg));
                if((sizee =recv(csocket, msg, sizeof(msg), 0)) <= 0) { //돌발 접속 중단
                    printf("aborted connect in receive\n");
                    tUsercnt--;
                    pthread_mutex_lock(&synclock);
                    session_clear(c);
                    pthread_mutex_unlock(&synclock);
                    continue;
                }
                memcpy(&rPsize[0], &msg[0], 2);
                pCnt = 0; // 패킷 카운터 초기화

                if(rPsize[0] == sizee) {
                    memcpy(&rEnd, &msg[rPsize[0] -2], 2);
                    if(rEnd != nEnd) {
                        printf("패킷의 끝이 잘못됨..\n");
                        continue;
                    }
                    pCnt++;
                } else if(rPsize[0] < sizee) {
                    divIdx = 0;
                    mx = 0;
                    for(;;) {
                        if(divIdx >= sizee) break; // 패킷 분리 완료
                        memcpy(&rPsize[pCnt], &msg[divIdx], 2); //패킷 길이 읽기
                        memset(msgto[pCnt], 0, MAX_PACKET_SIZE);
                        memcpy(&msgto[pCnt][0], &msg[divIdx], rPsize[pCnt]); //임시 버퍼에 저장
                        memcpy(&rEnd, &msgto[pCnt][rPsize[pCnt] - 2], 2);
                        if(rEnd != nEnd) { // 엔드 마크 체크, 값이 틀린 경우
                            printf("패킷의 끝이 잘못됨.(붙어온 패킷을 앞길이만큼 잘랐지만 잘못됨...)\n");
                            break;
                        } else {
                            mx += rPsize[pCnt]; // 다음번 패킷 시작 위치 계산
                            pCnt += 1;
                            divIdx = mx;
                        }

                    }
                } else continue;
                if(pCnt == 1) {
                    if (c->state == conn_state_empty) {
                        state = thr_beforeAuth(msg, c, conn, sockPair.udpSock);
                        if(state == 0){
                            printf("유저 캐릭터 로딩 실패\n");
                            c->state = conn_state_destroy;
                            continue;
                        }
                    } else if(c->state == conn_state_authed) {
                        state = main_act(msg, rPsize[0], c, conn, sockPair.udpSock);
                        if(state == 0) {
                            printf("유저 명령 처리 오류\n");
                            // c->state = conn_state_destroy;
                            continue;
                        }
                    } else continue;
                }else {
                    for(n = 0;n < pCnt; n++) {
                        if(c->state == conn_state_empty) {
                            state = thr_beforeAuth(msgto[n], c, conn, sockPair.udpSock);
                            if(state == 0) {
                                printf("유저 캐릭터 로딩 실패\n");
                                c->state = conn_state_destroy;
                                continue;
                            }
                        } else if(c->state ==conn_state_authed) {
                            state = main_act(msgto[n], rPsize[n], c, conn, sockPair.udpSock);
                            if(state == 0) {
                                printf("유저 명령 처리 오류\n");
                                // c->state = conn_state_destroy;
                                continue;
                            }                            
                        } else continue;
                    }
                }                
            } //if (FD_ISSET(csocket, &rfds)) end 연결 초기화 후의 패킷 수신

            if(FD_ISSET(csocket, &efds))
            {
                printf("abrut disconnect in efds\n");
                tUsercnt--;

                if(sizee==1) printf("DISCONNECT DATA SAVE OKOKOKOKOK\n");
                else printf("DISCONNECT DATA SAVE FALSEFALSEFALSE\n");

                pthread_mutex_lock(&synclock);
                session_clear(c);
                pthread_mutex_unlock(&synclock);
            }
        } // 연결 리스트 탐색 루프 끝
    } //무한루프 끝

    PQfinish(conn);
}

void * Thread_sendpack(void *arg)
{
    int i, j, ERRkey;
    int rets, tmp_sock;
    type_packet *packet;
    type_session *c;
    type_session *ERRmap[16];
    type_session *tmp;
    int que_len;

    for(i=0;i<16;i++)
        ERRmap[i] = NULL;

    for(;;) {
        ERRkey = 0;
        pthread_mutex_lock(&sendQuelock);
        que_len = queue_get_length((type_queue const * const *)&send_que);
        while(que_len == 0) { //큐의 길이가 0인 동안 데이터가 들어올 때까지 기다린다.
            pthread_cond_wait(&sendQuecond, &sendQuelock);
            que_len = queue_get_length((type_queue const * const *)&send_que);
        }
        packet = queue_pull_packet(&send_que); //큐에서 노드를 가져옴
        i = 0;
        while(i < que_len) {  // 큐에 들어온 데이터를 모두 전송할 때까지 루프를 돈다.
            i+=1;

            c = (type_session *)packet->session; //큐의 구조체에 있는 수신자의 연결 정보를 가져옴
            tmp_sock = c->sock;

            if(!packet->session) { //연결 정보가 NULL일 경우
                printf("세션이 널.............\n");
                packet_destroy(packet);
                if(i<que_len) packet = queue_pull_packet(&send_que); //보내지 않은 노드를 가져옴
            } else if(tmp_sock == 2) {
                printf("sigpipe 발생................\n");
                packet_destroy(packet);
                if(i < que_len) packet = queue_pull_packet(&send_que);
            } else {
                if(packet->size < 5){
                    printf("Send error 패킷 길이가 5보다 작다...%d\n", packet->data[2]);
                    rets = send(packet->cli_sock, packet->data, packet->size, 0);   //송신
                    if(rets <= 0){
                        printf("Send errorss\n", rets);
                        ERRmap[ERRkey] = c;
                        ERRkey++;
                    }
                }
                packet_destroy(packet);
                if(i < que_len) packet = queue_pull_packet(&send_que); 
                pthread_mutex_unlock(&sendQuelock);

                if(ERRkey != 0){ //송신중 오류가 발생한 클라이언트 처리
                    pthread_mutex_lock(&synclock);
                    for(j=0; j<ERRkey;j++) {
                        ERRmap[j]->sock = signal_sock_no;
                        ERRmap[j]->state = conn_state_destroy;
                        tmp=ERRmap[j];
                        ERRmap[j] = NULL;
                    }
                    pthread_mutex_unlock(&synclock);
                }

            }
        }
    }
}

void * Thread_udpserv(void *arg)
{

}

void setup_signals()
{

}

