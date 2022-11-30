#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/blkdev.h>
#include <linux/genhd.h>

#include "ioctl.h"

// #define MY_IOCTL_IN _IOC(_IOC_WRITE, 'k', 1, sizeof(my_ioctl_data))
#define MY_MAJOR       42
#define MY_MAX_MINORS  1


#define MY_BLOCK_MAJOR           240
#define MY_BLKDEV_NAME          "CLB"
#define NR_SECTORS                   1024
#define KERNEL_SECTOR_SIZE           512

MODULE_DESCRIPTION("UwUTemp");
MODULE_AUTHOR("Me");
MODULE_LICENSE("GPL");



struct bio {
    //...
    struct gendisk          *bi_disk;
    unsigned int            bi_opf;         /* bottom bits req flags, top bits REQ_OP. Use accessors. */
    //...
    struct bio_vec          *bi_io_vec;     /* the actual vec list */
    //...
    struct bvec_iter        bi_iter;
    // ...
    void                    *bi_private;
    //...
};

static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
static int my_open(struct inode *inode, struct file *file);
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset);
static int my_release(struct inode *, struct file *);
static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg);
static int my_block_init(void);
static void my_block_exit(void);
static int create_block_device(struct my_block_dev *dev);
static int my_block_init(void);
static void delete_block_device(struct my_block_dev *dev);
static void my_block_exit(void);
static int my_block_open(struct block_device *bdev, fmode_t mode);
static int my_block_release(struct gendisk *gd, fmode_t mode);
static blk_status_t my_block_request(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data *bd);
static void my_block_transfer(struct my_block_dev *dev, size_t start,
                              size_t len, char *buffer, int dir);
static int my_xfer_bio(struct my_block_dev *dev, struct bio *bio);

struct blk_mq_tag_set {
  // ...
  const struct blk_mq_ops   *ops;
  unsigned int               nr_hw_queues;
  unsigned int               queue_depth;
  unsigned int               cmd_size;
  int                        numa_node;
  void                      *driver_data;
  struct blk_mq_tags       **tags;
  struct list_head           tag_list;
  // ...
};

static struct my_block_dev {
    spinlock_t lock;                /* For mutual exclusion */
    struct request_queue *queue;    /* The device request queue */
    struct gendisk *gd;             /* The gendisk structure */
    struct blk_mq_tag_set tag_set;
    //...
} dev;



struct my_device_data {
    struct cdev cdev;

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
    .unlocked_ioctl = my_ioctl

};

const struct block_device_operations {
    .owner = THIS_MODULE,
    .open = my_block_open,
    .release = my_block_release
}

int init_module(void) {
    // pr_warn("--------\n");
    int i, err;

    err = register_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS,
                                 "my_device_driver");
    if (err != 0) {
        // pr_warn("driver init fail\n");
        return err;
    }

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* initialize devs[i] fields */
        cdev_init(&devs[i].cdev, &my_fops);
        cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), 1);
    }
    // pr_warn("driver init succes\n");
    return 0;
}

void cleanup_module(void) {
    int i;

    for(i = 0; i < MY_MAX_MINORS; i++) {
        /* release devs[i] fields */
        cdev_del(&devs[i].cdev);
    }
    unregister_chrdev_region(MKDEV(MY_MAJOR, 0), MY_MAX_MINORS);
    // pr_warn("driver close\n");
}

/* my_open, my_read etc. to implement */
static int my_read(struct file *file, char __user *user_vuffer, size_t size, loff_t *offset) {
    struct my_device_data *my_data;

    my_data = (struct my_device_data *) file->private_data;
    

    return (0);
}

static int my_open(struct inode *inode, struct file *file) {
    struct my_device_data *my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

    /* validate access to device */
    file->private_data = my_data;
    
    return 0;
}
static int my_release(struct inode *inode, struct file *file) {
    return 0;
}
/** write
    // static int my_write(struct file *file, const char __user *user_buffer,size_t size, loff_t * offset)
    // {
    //     struct my_device_data *my_data = (struct my_device_data *) file->private_data;
    //     ssize_t len = min(my_data->size - *offset, size);

    //     if (len <= 0)
    //         return 0;

    //     if (copy_from_user(my_data->buffer + *offset, user_buffer, len))
    //         return -EFAULT;

    //     *offset += len;
    //     return len;
    // }
**/
static long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg) {
    struct my_device_data *my_data = (struct my_device_data*) file->private_data;
    struct my_ioctl_data mid;

    switch(cmd) {
    case MY_IOCTL_IN:
        
        break;
    default:
        return -ENOTTY;
    }

    return 0;
}

static int my_block_init(void) {
    int status;

    status = register_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
    if (status < 0) {
        printk(KERN_ERR "unable to register mybdev block device\n");
        return -EBUSY;
    }
}

static void my_block_exit(void) {
     //...
     unregister_blkdev(MY_BLOCK_MAJOR, MY_BLKDEV_NAME);
}

static struct blk_mq_ops my_queue_ops = {
   .queue_rq = my_block_request,
};

static int create_block_device(struct my_block_dev *dev) {
    /* Initialize tag set. */
    dev->tag_set.ops = &my_queue_ops;
    dev->tag_set.nr_hw_queues = 1;
    dev->tag_set.queue_depth = 128;
    dev->tag_set.numa_node = NUMA_NO_NODE;
    dev->tag_set.cmd_size = 0;
    dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
    err = blk_mq_alloc_tag_set(&dev->tag_set);
    if (err) {
        goto out_err;
    }

    /* Allocate queue. */
    dev->queue = blk_mq_init_queue(&dev->tag_set);
    if (IS_ERR(dev->queue)) {
        goto out_blk_init;
    }

    blk_queue_logical_block_size(dev->queue, KERNEL_SECTOR_SIZE);

     /* Assign private data to queue structure. */
    dev->queue->queuedata = dev;
    
    /* Initialize the gendisk structure */
    dev->gd = alloc_disk(MY_BLOCK_MINORS);
    if (!dev->gd) {
        printk (KERN_NOTICE "alloc_disk failure\n");
        return -ENOMEM;
    }

    dev->gd->major = MY_BLOCK_MAJOR;
    dev->gd->first_minor = 0;
    dev->gd->fops = &my_block_ops;
    dev->gd->queue = dev->queue;
    dev->gd->private_data = dev;
    snprintf (dev->gd->disk_name, 32, "myblock");
    set_capacity(dev->gd, NR_SECTORS);

    add_disk(dev->gd);

out_blk_init:
    blk_mq_free_tag_set(&dev->tag_set);
out_err:
    return -ENOMEM;
}

static int my_block_init(void) {
    int status;
    //...
    status = create_block_device(&dev);
    if (status < 0)
        return status;
    //...
}

static void delete_block_device(struct my_block_dev *dev) {
    if (dev->gd)
        del_gendisk(dev->gd);
    //...
    blk_mq_free_tag_set(&dev->tag_set);
    blk_cleanup_queue(dev->queue);
}

static void my_block_exit(void) {
    delete_block_device(&dev);
    //...
}

static int my_block_open(struct block_device *bdev, fmode_t mode)
{
    //...

    return 0;
}

static int my_block_release(struct gendisk *gd, fmode_t mode)
{
    //...

    return 0;
}

static blk_status_t my_block_request(struct blk_mq_hw_ctx *hctx,
                                     const struct blk_mq_queue_data *bd)
{
    struct request *rq = bd->rq;
    struct my_block_dev *dev = q->queuedata;

    blk_mq_start_request(rq);

    if (blk_rq_is_passthrough(rq)) {
        printk (KERN_NOTICE "Skip non-fs request\n");
        blk_mq_end_request(rq, BLK_STS_IOERR);
        goto out;
    }

    /* do work */
    ...

    blk_mq_end_request(rq, BLK_STS_OK);

out:
    return BLK_STS_OK;
}

static int my_xfer_bio(struct my_block_dev *dev, struct bio *bio)
{
    struct bio_vec bvec;
    struct bvec_iter i;
    int dir = bio_data_dir(bio);

    /* Do each segment independently. */
    bio_for_each_segment(bvec, bio, i) {
        sector_t sector = i.bi_sector;
        char *buffer = kmap_atomic(bvec.bv_page);
        unsigned long offset = bvec.bv_offset;
        size_t len = bvec.bv_len;

        /* process mapped buffer */
        my_block_transfer(dev, sector, len, buffer + offset, dir);

        kunmap_atomic(buffer);
    }

    return 0;
}