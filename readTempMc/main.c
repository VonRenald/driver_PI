#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ioctl.h"

struct my_data {
    char* word;
    int len_word;
};

int main(int argc, char** argv)
{
    // if (argc != 2)
    // {
    //     printf("sudo %s <size len>\n",argv[0]);
    //     return 1;
    // }

    // int size = abs(atoi(argv[1]));

    int file = open("/dev/TpTemp", O_RDWR);
    
    if(file < 0)
    {
        perror("open");
        exit(errno);
    }
    
    int size = 255;
    for(int i = 0; i< 10; i++)
    {
        char* tab;
        tab = (char *) malloc( (size) * sizeof(char) ); 
        int ret = 0;
        ret = read(file, tab, size);
        printf("ret : %d \n%s\n",ret,tab);
        free(tab);
        sleep(2);
    }



    // struct my_data* data;
    // struct my_data* test;
    // test = (struct my_data*) malloc( sizeof(struct my_data) ); 
    // data = (struct my_data*) malloc( sizeof(struct my_data) ); 
    // test->word = (char*) malloc(sizeof(char));
    // test->word[0] = 'k';
    // test->len_word = 1;
    // data->word = (char *) malloc( 3 * sizeof(char) ); 
    // data->word[0] = 'F';
    // data->word[1] = 'S';
    // data->word[2] = 'I';
    // data->len_word = 3;
    // // data->len_word = 3;
    // printf("ok1\n");





    
    // ioctl(file,MY_IOCTL_IN , (unsigned long) data);
    // printf("ok2\n");
    // ret = read(file, tab, size);
    // printf("ret : %d %s\n",ret,tab);


    // ioctl(file,MY_IOCTL_IN_TEMP , (unsigned long) data);


    // ret = read(file, tab, size);
    // printf("ret : %d %d\n",ret,tab[0]);

    // ioctl(file,MY_IOCTL_IN_STRING , (unsigned long) data);

    // ret = read(file, tab, 15);
    // printf("ret : %d %s\n",ret,tab);

    // close(file);
    // free(tab);
    // free(test->word);
    // free(test);
    // free(data->word);
    // free(data);
    
    close(file);

    

    return 0;
}
