#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include "svr.h"

#define GAME_FRAME 	0x0F
#define _ONE_SEC	GAME_FRAME
#define AFRAME	900000/GAME_FRAME

int svr_ret_tot() {
    return 0;
}


// 유저의 움직임, 자동 저장, 체력 회보ㄱ, 공격 시간 텀등 자동적으로 적용되는 캐릭터의 카운트와 관련된 일을 처리한다.
void * Thread_sync(void *arg) {
    struct timeval start, stop;
    long tmp, utmp;
    short regenTime, Half_sec;
    int i, maxNo;

    sleep(1);

    regenTime = _ONE_SEC*30; //리젠 타임
    Half_sec = _ONE_SEC/2;
    for(;;) {
        gettimeofday(&start, NULL); //시작 시간
        pthread_mutex_lock(&synclock);
        maxNo = svr_ret_tot(); //총 접속자수 
        for(i=0;i<maxNo;i++){

        }
        pthread_cond_signal(&synccond);
        pthread_mutex_unlock(&synclock);

        gettimeofday(&stop, NULL); //종료 시간
        tmp = stop.tv_sec - start.tv_sec;
        utmp = stop.tv_usec - start.tv_usec;
        //프레임에 걸리는 시간보다 작을 경우 남은 시간만큼 멈춤
        if(tmp>1) { //1frame 0.033초 33000, 1

        } else if(tmp==1) {
            utmp = 1000000 + utmp; // 걸린 시간
            if(utmp<AFRAME){
                stop.tv_sec = 0;
                stop.tv_usec = AFRAME-utmp;
                select(0, NULL, NULL, NULL, &stop); //남은 시간만큼 멈춤
            }
        } else {
            if(utmp<AFRAME) {
                stop.tv_sec = 0;
                stop.tv_usec = AFRAME - utmp;
                select(0, NULL, NULL, NULL, &stop); //남은 시간만큼 멈춤
            }
        }
    } //for 루프 끝
}