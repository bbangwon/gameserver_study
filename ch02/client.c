#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "packet.h"
#include "publicInc.h"

#define MAXLINE 1024

char *escapechar = "/exit";
char *comm_loc = "/where";
char *comm_mv = "/move";
char *comm="/";

char name[32];
int main(int argc, char *argv[]) {
    unsigned char message[MAXLINE], messa[8][MAXLINE];
    unsigned char mesg[MAXLINE];
    char *tmp;
    unsigned short nEnd = PEND, rEnd;
    struct sockaddr_in server_addr;
    int maxfdp1, ulen;
    int s;
    fd_set read_fds;
    int kkkkk, len=1, charlen, tot, res, i, j, mx;
    char uname[32], chatmsg[MAXLINE];

    int Lox, Loz, tx, tz, pCnt;
    short userNo, tmpNo, msgLen, cnt, klen, divsize, rPsize[8];

    int userStatus = 0;

    char *servip="127.0.0.1";
    int porNo = 9901;
    if (argc != 3) {
        printf("사용법: %s name password\n", argv[0]);
        exit(0);
    }

    // 소켓생성
    if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Client : Can't open stream socket.\n");
        exit(0);
    }

    // bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(servip);
    server_addr.sin_port = htons(porNo);

    //연결 요청
    if(connect(s, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        printf("Client : Can't connect to server.\n");
        exit(0);
    }else {
        printf("서버에 접속되었습니다. \n");
    }

    maxfdp1 = s+1;
    memset(message,0,MAXLINE);
    len=2;

    message[len] = PK_USER_AUTH;
    len+=1;
    
    charlen = strlen(argv[1]);    
    message[len]=charlen;
    len+=1;
    
    memcpy(&message[len], &argv[1][0], charlen);
    len+=charlen;

    charlen = strlen(argv[2]);
    message[len] = charlen;
    len+=1;

    memcpy(&message[len], &argv[2][0], charlen);
    len+=charlen;

    memcpy(&message[len], &nEnd, 2);
    len+=2;

    memcpy(&message[0], &len, 2);
    if(send(s, message, len, 0) < 0) {
        close(s);
        printf("Error : Write error on socket.\n");
        exit(0);
    }

    while(1) {
        memset(message, 0, MAXLINE);
        FD_ZERO(&read_fds);
        FD_SET(0, &read_fds);
        FD_SET(s, &read_fds);

        if(select(maxfdp1, &read_fds, (fd_set *)0, (fd_set *)0, (struct timeval *)0) <0) {
            printf("select error\n");
            exit(0);
        }
        if(FD_ISSET(s, &read_fds)) {
            ssize_t size;
            if((size = recv(s, message, MAXLINE, 0)) > 0) {
                memcpy(&rPsize[0], &message[0], 2);
                if(size==rPsize[0]) pCnt=1;
                else {
                    pCnt = 0;
                    divsize = 0;
                    mx=0;
                    for(;;){
                        if(mx>=size) break;
                        memset(messa[pCnt], 0, MAX_PACKET_SIZE);
                        memcpy(&messa[pCnt][0], &message[divsize], rPsize[pCnt]);
                        memcpy(&rEnd, &messa[pCnt][rPsize[pCnt] - 2], 2);
                        if(rEnd != nEnd){
                            printf("패킷의 끝이 잘못됨..(붙어온 패킷을 앞 길이만큼 잘랐지만 잘못됨..)\n");
                            break;
                        }else {
                            divsize = rPsize[pCnt];
                            pCnt += 1;
                            memcpy(&rPsize[pCnt], &message[divsize], 2);
                            mx+=divsize;
                        }
                    }
                }
                for(j=0;j<pCnt;j++){
                    if(j>0) memcpy(&message[0], &messa[j][9], rPsize[j]);
                    kkkkk = message[2];
                    switch(kkkkk) {
                        case PK_USER_AUTH:
                            tot=message[3];
                            ulen=message[4];
                            memcpy(&name[0], &message[5], ulen);
                            name[ulen]='\0';
                            memcpy(&userNo, &message[5+ulen], 2);
                            printf("인증이 성공함 %s(userNo: %d) 총원:%d\n", name, userNo, tot);
                            userStatus = 1;
                            fflush(NULL);
                            break;
                        case PK_USER_INFO:
                            memcpy(&cnt, &message[3], 2);
                            for(i=0,klen=5;i<cnt;i++){
                                memcpy(&tmpNo, &message[klen], 2);
                                klen+=2;
                                tot = message[klen];
                                klen +=1;
                                memcpy(&name[0], &message[klen], tot);
                                klen += tot;
                                name[tot]='\0';
                                tx=message[klen];
                                klen += 1;
                                tz=message[klen];
                                klen+=1;
                                printf("유저정보 %s(userNo:%d) 위치:%d,%d\n", name, tmpNo, tx, tz);
                            }
                            fflush(NULL);
                            break;
                        case PK_REQ_LOC:
                            Lox=message[3];
                            Loz=message[4];
                            printf("클라이언트 %s의 위치 X:%d, Z:%d\n", name, Lox, Loz);
                            fflush(NULL);
                            break;
                        case PK_USER_CHAT:
                            tot = message[3];
                            memcpy(&uname[0], &message[4], tot);
                            uname[tot]='\0';
                            tot+=4;
                            ulen=message[tot];
                            memcpy(&msgLen, &message[tot], 2);
                            memcpy(&chatmsg[0], &message[tot+2], ulen);
                            chatmsg[ulen]='\0';
                            printf("chat msg [%s]: %s", uname, chatmsg);
                            fflush(NULL);
                            break;
                        case PK_USER_MV:
                            res = message[3];
                            memcpy(&tmpNo, &message[4], 2);
                            tot = message[6];
                            memcpy(&uname[0], &message[7], tot);
                            uname[tot]='\0';

                            Lox=message[tot+7];
                            Loz=message[tot+8];
                            tx=message[tot+9];
                            tz=message[tot+10];

                            if(res==RET_TRUE){
                                if(tmpNo==userNo) {
                                    printf("USER_MOVE %d, %d에서 %d, %d로 이동이 성공하였음\n", tx, tz, Lox, Loz);
                                } else {
                                    printf("USER_MOVE %s가 %d, %d에서 %d, %d로 이동함\n", uname, tx, tz, Lox, Loz);
                                }
                            } else {
                                if(tmpNo==userNo) {
                                    printf("USER_MOVE %d, %d에서 %d, %d로 이동이 실패하였음\n", Lox, Loz, tx, tz);
                                } else {
                                    printf("USER_MOVE %s가 %d, %d에서 %d, %d로 이동이 실패함\n", uname, Lox, Loz, tx, tz);
                                }
                            }
                            fflush(NULL);
                            break;
                    }
                }
            }
        }
        if(FD_ISSET(0, &read_fds)) {
            memset(message, 0, 1024);
            if(fgets(message, MAXLINE, stdin)) {
                len=2;
                memset(mesg, 0, 1024);
                if(userStatus==1){
                    if(!strncmp(comm, message, 1)){
                        tot=strlen(message)-1;
                        if(tot==6) {
                            if(!strncmp(comm_loc, message, 6)){
                                mesg[len]=PK_REQ_LOC;
                                len+=1;
                                memcpy(&mesg[len], &nEnd, 2);
                                len+=2;
                                memcpy(&mesg[0], &len, 2);
                            }
                        }else {
                            if(!strncmp(comm_mv, message, 5)) {
                                tmp=strtok(message, " ");
                                tx=atoi(strtok(NULL, " "));
                                tz=atoi(strtok(NULL, " "));
                                mesg[len] = PK_CMD_MV;
                                len+=1;
                                mesg[len]=tx;
                                len+=1;
                                mesg[len]=tz;
                                len+=1;
                                memcpy(&mesg[len], &nEnd, 2);
                                len+=2;
                                memcpy(&mesg[0], &len, 2);
                                printf("move %d,%d\n", tx, tz);
                            }
                        }
                        printf("입력길이:%d\n", tot);
                    } else {
                        msgLen=strlen(message);
                        mesg[len]=PK_USER_CHAT;
                        len+=1;
                        memcpy(&mesg[len], &msgLen, 2);
                        len+=2;
                        if(tot>=988) {
                            memcpy(&mesg[len], &message[0], 988);
                            len+=988;
                            memcpy(&mesg[len], &nEnd, 2);
                            len+=2;
                        } else {
                            memcpy(&mesg[len], &message[0], msgLen);
                            len+=tot;
                            memcpy(&mesg[len], &nEnd, 2);
                            len+=2;
                        }
                        memcpy(&mesg[0], &len, 2);
                    }
                } else {
                    printf("userstatus not!!!\n");
                }
                if(len>4){
                    if(send(s, mesg, len, 0) <0)
                        printf("Error : Write error on socket.\n");
                    
                    if(strstr(message, escapechar) != NULL) {
                        printf("Good bye.\n");
                        close(s);
                        exit(0);
                    }
                }
            }
        }
    }

    




}