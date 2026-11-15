//기본 스레드와 소켓 I/O를 다루는 함수
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

#define _G_TCP_PORT 9901
#define _G_UDP_PORT 8801

#define LISTEN_QUEUE 0x0A

typedef struct sockPair {
    int tcpSock;
    int udpSock;
} type_sockpair;

void * Thread_recvpack(void *arg);
void * Thread_sendpack(void *arg);
void * Thread_sync(void *arg);
void * Thread_udpserv(void *arg);

void init_t_sessionArray();
void setup_signals();


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

}

void * Thread_sendpack(void *arg)
{

}

void * Thread_sync(void *arg)
{

}

void * Thread_udpserv(void *arg)
{

}

void init_t_sessionArray()
{

}

void setup_signals()
{

}