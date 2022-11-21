#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
// #include <asm/ioctl.h>

// #define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(my_ioctl_data))
#define MY_MAJOR       42
#define MY_MAX_MINORS  1


MODULE_DESCRIPTION("UwU");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");

// static int dummy_init(void){
//     pr_warn("Hi\n");
//     return 0;
// }
// static void dummy_exit(void){
//     pr_warn("Bye\n");
// }

// module_init(dummy_init);
// module_exit(dummy_exit);

static int my_open(struct inode *inode, struct file *file);
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset);
static int my_release(struct inode *, struct file *);
// static int my_write(struct file *file, const char __user *user_buffer,size_t size, loff_t * offset);
// static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);

struct my_device_data {
    struct cdev cdev;
    int size;
    char buffer[10];
    /* my data starts here */
    //...
};

struct my_device_data devs[MY_MAX_MINORS];

const struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .read = my_read,
    // .write = my_write,
    .release = my_release,
    // .unlocked_ioctl = my_ioctl
};

int init_module(void)
{
    int i, err;

    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS,
                                 "my_device_driver");
    if (err != 0) {
        pr_warn("driver init fail\n");
        return err;
    }

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* initialize devs[i] fields */
        cdev_init(&devs[i].cdev, &my_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    }
    pr_warn("driver init succes\n");
    return 0;
}

void cleanup_module(void)
{
    int i;

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* release devs[i] fields */
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
    pr_warn("driver close\n");
}

/* my_open, my_read etc. to implement */
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset)
{
    struct my_device_data *my_data;

    my_data = (struct my_device_data *) file->private_data;
    char secil[5] = {'S','E','C','I','L'};
    int i=0;
    while(i<size){
        user_vuffer[i] = secil[i%5];
        i++;
    }
    user_vuffer[i] = '\0';
    return (size+1);
}

static int my_open(struct inode *inode, struct file *file)
{
    struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

    /* validate access to device */
    file->private_data = my_data;

    /* initialize device */
    // ...

    return 0;
}
static int my_release(struct inode *inode, struct file *file)
{
    return 0;
}

// static int my_write(struct file *file, const char __user *user_buffer,size_t size, loff_t * offset)
// {
//     struct my_device_data *my_data = (struct my_device_data *) file->private_data;
//     ssize_t len = min(my_data->size - *offset, size);

//     if (len <= 0)
//         return 0;

//     /* read data from user buffer to my_data->buffer */
//     if (copy_from_user(my_data->buffer + *offset, user_buffer, len))
//         return -EFAULT;

//     *offset += len;
//     return len;
// }

// static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
// {
//     struct my_device_data *my_data =
//          (struct my_device_data*) file->private_data;
//     my_ioctl_data mid;

//     switch(cmd) {
//     case MY_IOCTL_IN:
//         if( copy_from_user(&mid, (my_ioctl_data *) arg,
//                            sizeof(my_ioctl_data)) )
//             return -EFAULT;

//         /* process data and execute command */

//         break;
//     default:
//         return -ENOTTY;
//     }

//     return 0;
// }