#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "ioctl.h"

struct my_data {
    char* word;
    int len_word;
};

int main(int argc, char** argv)
{

    if (argc != 2)
    {
        printf("sudo %s <size len>\n",argv[0]);
        return 1;
    }

    int size = abs(atoi(argv[1]));

    int file = open("/dev/TpRead", O_RDWR);
    
    if(file < 0)
    {
        perror("open");
        exit(errno);
    }
    
    bool stop = false;
    int ret = 0;
    char* tab;
    struct my_data* data;
      

    tab = (char *) malloc( (size+1) * sizeof(char) ); 
    
    ret = read(file, tab, size);
    printf("ret : %d %s\n",ret,tab);

    
    data = (struct my_data*) malloc( sizeof(struct my_data) ); 
    data->word = (char *) malloc( 3 * sizeof(char) ); 
    data->word[0] = 'F';
    data->word[1] = 'S';
    data->word[2] = 'I';
    data->len_word = 3;

    
    ioctl(file,MY_IOCTL_IN , (unsigned long) data);
    ret = read(file, tab, size);
    printf("ret : %d %s\n",ret,tab);



    
    free(tab);
    free(data->word);
    free(data);

    
    close(file);
    

    return 0;
}
