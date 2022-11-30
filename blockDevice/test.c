#include <stdio.h>

int main()
{
    char s1[10] = "test1";
    char s2[10] = "test2";

    printf("-> %s %s\n",s1,s2);

    char s3[20];
    sprintf(s3,"%s-> %s",s1,s2);
    printf("%s\n",s3);
    sprintf(s3,"%s . %s",s3,s1);
    printf("%s\n",s3);

    return 0;
}