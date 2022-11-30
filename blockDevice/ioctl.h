

#include <asm/ioctl.h>
#define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(struct my_ioctl_data))
#define MY_IOCTL_IN_TEMP _IOC(_IOC_WRITE, 't', 1, sizeof(struct my_ioctl_data))
#define MY_IOCTL_IN_STRING _IOC(_IOC_WRITE, 't', 2, sizeof(struct my_ioctl_data))




struct my_ioctl_data {
    char* word;
    int len_word;
};

