#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <pthread.h>
#include <libpq-fe.h>

#include "session.h"
#include "thr_act.h"
#include "publicInc.h"

typedef struct sockPair{
	int tcpSock;
	int udpSock;
} type_sockpair;

int tUsercnt			= 0;

pthread_mutex_t 		synclock		= PTHREAD_MUTEX_INITIALIZER;

void main()
{
    
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
