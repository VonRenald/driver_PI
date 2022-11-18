#include <stdio.h>

int main()
{
    unsigned char a = 124;
    unsigned char b = 1;
    unsigned char a2;


    a2 = a & 0xb00001111;
    unsigned char signe = b & 0xb10000000;
    b = b & 0xb00000111;

    a = a >> 4;
    b = b << 4;
    int temp = a+b;


    printf("signe : %d \n",signe);
    printf("temp : %d \n",temp);
    unsigned char text[10];
    sprintf(text,"%d",temp);
    printf("%s\n",text);

    a2 = 8+4+2+1;
    printf("a2 : %d\n",a2);
    
    float f = 0.0f;

    f+= ((a2 & 8) == 8)? 1.0/2.0  : 0.0;
    f+= ((a2 & 4) == 4)? 1.0/4.0  : 0.0;
    f+= ((a2 & 2) == 2)? 1.0/8.0  : 0.0;
    f+= ((a2 & 1) == 1)? 1.0/16.0 : 0.0;
    printf("float : %f\n",f);
    f+= temp;
    printf("float : %f\n",f);
    sprintf(text,"%f",f);
    printf("%s\n",text);


    // for(int i = 0;i<10;i++)
    // {
    //     printf("%c ",text[i]);
    //     if(text[i] == '\0') {printf("o");}
    // }
    // printf("\n");

    // char a = 124;
    // int temp = a >> 4;
    // char b = 1;
    // for (int i = 4; i <= 7;i++)
    // {
    //     printf("temps : %d\n",temp);
    //     if (i < 7){
    //         temp += (b & 0b00000001) * 2 <<i;
    //         b = b >> 1;
    //     }
    //     else
    //     {
    //        if(b & 0b00000001 == 1)
    //        {
    //         temp = -temp;
    //        } 
    //     }
    // } 
    
    // printf("%d %d %d\n",a,b,temp);

}