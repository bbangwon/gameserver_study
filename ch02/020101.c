#include <stdio.h>

typedef struct padTest1
{ //패딩을 고려하지 않고, 구조체 멤버 변수 나열
    char oneByte0;
    int fourByte0;
    char oneByte1;
    int fourByte1;
    char oneByte2;
    int fourByte2;
    char oneByte3;
    int fourByte4;        
} tst1;

typedef struct padTest2
{ //패딩을 고려하여 구조체 멤버 변수 나열
    char oneByte0;
    char oneByte1;
    char oneByte2;
    char oneByte3;
    int fourByte0;
    int fourByte1;
    int fourByte2;
    int fourByte4;
} tst2;

int main() 
{
    tst1 paddings0;
    tst2 paddings1;

    printf("tst1 size:%d\n", sizeof(paddings0));
    printf("tst2 size:%d\n", sizeof(paddings1));
}
