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

    int file = open("/dev/TpTemp", O_RDWR);
    
    if(file < 0)
    {
        perror("open");
        exit(errno);
    }
    

    
    close(file);

    

    return 0;
}
