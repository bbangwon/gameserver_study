#include <stdio.h>
#include <string.h>
#include <netinet/in.h>

int main() {
    unsigned char Little_Endian_data[6];
    unsigned char Big_Endian_data[6];
    unsigned short U_short = 234, U_Netshort;
    unsigned int U_int = 987, U_Netint;

    printf("Little endian U_short:%d U_int:%d\n\n", U_short, U_int);

    U_Netshort = htons(U_short); //네트워크 바이트 순서로 변환(빅 엔디안)
    U_Netint = htonl(U_int); //네트워크 바이트 순서로 변환(빅 엔디안)

    printf("Big Endian U_Netshort:%d U_Netint:%d\n", U_Netshort, U_Netint);

    memcpy(&Little_Endian_data[0], &U_short, 2);
    memcpy(&Little_Endian_data[2], &U_int, 4);

    Big_Endian_data[0] = Little_Endian_data[1];
    Big_Endian_data[1] = Little_Endian_data[0];

    Big_Endian_data[2] = Little_Endian_data[5];
    Big_Endian_data[3] = Little_Endian_data[4];
    Big_Endian_data[4] = Little_Endian_data[3];
    Big_Endian_data[5] = Little_Endian_data[2];

    memcpy(&U_Netshort, &Big_Endian_data[0], 2);
    memcpy(&U_Netint, &Big_Endian_data[2], 4);
    printf("Big endian U_Netshort:%d U_Netint:%d\n", U_Netshort, U_Netint);
}