

#include <asm/ioctl.h>
#define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(struct my_ioctl_data))




struct my_ioctl_data {
    char* word;
    int len_word;
};

