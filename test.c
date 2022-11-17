#include <stdio.h>

int main()
{
    char a = 124;
    int temp = a >> 4;
    int b = 1;
    for (int i = 4; i <= 7;i++)
    {
        if (i < 7){
            temp += (b & 0b0000001) * 2<<i;
        }
    } 
    
    printf("%d %d %d\n",a,b,temp);

}