#include <stdio.h>

int main()
{
    char a = 124;
    char b = 1;

    char signe = b & 0xb10000000;
    b = b & 0xb00000111;

    a = a >> 4;
    b = b << 4;
    int temp = a+b;


    printf("signe : %d \n",signe);
    printf("temp : %d \n",temp);
    char text[10];
    sprintf(text,"%d",temp);
    printf("%s\n",text);
    for(int i = 0;i<10;i++)
    {
        printf("%c ",text[i]);
        if(text[i] == '\0') {printf("o");}
    }
    printf("\n");

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