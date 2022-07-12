#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);

struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read = scull_read,
    .write = scull_write,
    .open = scull_open,
    .release = scull_release,
};

loff_t scull_llseek(struct file *filep, loff_t off, int whence)
{
}

ssize_t scull_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
}

ssize_t scull_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
}

int scull_open(struct inode *inode, struct file *filep)
{
}

int scull_release(struct inode *inode, struct file *filep)
{
}

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops
    err = cdev_add(&dev->cdev, devno, 1);

    if(err) {
        printk(KERN_NOTICE "Error %d adding scull%d", err, index);
    }
}

static int __init initialization(void)
{
    int result;
    dev_t devno = 0;

    if (scull_major)
    {
        devno = MKDEV(scull_major, scull_minor);
        result = register_chrdev_region(devno, scull_nr_devs, "scull");
    }
    else
    {
        result = alloc_chrdev_region(devno, scull_minor, scull_nr_devs, "scull");
        scull_major = MAJOR(devno);
    }

    if (result < 0)
    {
        printk(KERN_WARNING "scull: can't get a majour %d\n", scull_major);
        return result;
    }

    return result;
}

static void __exit cleanup(void)
{
    unregister_chrdev_region(device, 4);
}

module_init(initialization);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Character Utility for Loading Localities")
MODULE_AUTHOR("Kosta S.");