#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("sudo %s <size len>\n",argv[0]);
        return 1;
    }

    int size = abs(atoi(argv[1]));

    int file = open("/dev/TP1", O_RDWR);
    
    if(file < 0)
    {
        perror("open");
        exit(errno);
    }
    
    
    char* tab;
    tab = (char *) malloc( (size+1) * sizeof(char) ); 
    int ret = read(file, tab, size);
    printf("ret : %d %s\n",ret,tab);
    close(file);
    free(tab);
    
    return 0;
}
