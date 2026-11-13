#include <stdio.h>
#include <string.h>

int main() 
{
    char packet[6];
    char cyper[6];
    short A=34, B=45, C=56, SECRETKEY=1234;
    short tmpA, tmpB, tmpC;
    int i;

    memcpy(&packet[0], &A, 2);
    memcpy(&packet[2], &B, 2);
    memcpy(&packet[4], &C, 2);

    printf("암호화전 바이너리데이터: ");
    for(i=0;i<6;i++) printf("%d ", packet[i]);
    printf("\n\n");

    printf("암호화후 바이너리데이터: ");
    for(i=0;i<6;i++) {
        cyper[i] = packet[i] ^ SECRETKEY;
        printf("%d ", cyper[i]);
    }
    printf("\n\n");

    memcpy(&tmpA, &cyper[0], 2);
    memcpy(&tmpB, &cyper[2], 2);
    memcpy(&tmpC, &cyper[4], 2);
    printf("암호화된 패킷에서 변수값을 추출했을 경우 A: %d, B: %d, C: %d\n", tmpA, tmpB, tmpC);

    printf("복호화후 바이너리 데이터: ");
    for(i=0;i<6;i++) {
        packet[i] = cyper[i] ^ SECRETKEY;
        printf("%d ", packet[i]);
    }
    printf("\n\n");

    memcpy(&A, &packet[0], 2);
    memcpy(&B, &packet[2], 2);
    memcpy(&C, &packet[4], 2);
    printf("복호화된 패킷에서 변수값을 추출했을 경우 A: %d, B: %d, C: %d\n", A, B, C);
    return 0;
}