#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include "scull.h"

int scull_major = SCULL_MAJOR;
int scull_minor = 0;
int scull_nr_devs = SCULL_NR_DEVS;
int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;

module_param(scull_major, int, S_IRUGO);
module_param(scull_minor, int, S_IRUGO);
module_param(scull_nr_devs, int, S_IRUGO);
module_param(scull_quantum, int, S_IRUGO);
module_param(scull_qset, int, S_IRUGO);

struct file_operations scull_fops = {
    .owner = THIS_MODULE,
    .llseek = scull_llseek,
    .read = scull_read,
    .write = scull_write,
    .open = scull_open,
    .release = scull_release,
};

int scull_trim(struct scull_dev *dev)
{
    struct scull_qset *next, *dptr;
    int qset = dev->qset;
    int i;

    for (dptr = dev->data; dptr; dptr = next)
    {
        if (dptr->data)
        {
            for (i = 0; i < qset; i++)
                kfree(dptr->data);

            kfree(dptr->data);
            dptr->data = NULL;
        }

        next = dptr->next;
        kfree(dptr);
    }

    dev->size = 0;
    dev->quantum = scull_quantum;
    dev->qset = scull_qset;
    dev->data = NULL;
    return 0;
}

struct scull_qset *scull_follow(struct scull_dev *dev, int n)
{
    struct scull_qset *qs = dev->data;

    if(!qs)
    {
        qs = dev->data = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
        if(qs == NULL)
            return NULL;
        memset(qs, 0, sizeof(struct scull_qset));
    }

    while(n--) {
        if(!qs->next) {
            qs->next = kmalloc(sizeof(struct scull_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;
			memset(qs->next, 0, sizeof(struct scull_qset));
        }
        qs = qs->next;
        continue;
    }

    return qs;
}

loff_t scull_llseek(struct file *filep, loff_t off, int whence)
{
    return 0; 
}

ssize_t scull_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filep->private_data;
    struct scull_qset *dptr;

    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset; // Amount of bytes in the list
    int item, s_pos, q_pos, rest;
    ssize_t retval = 0;

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;
    if (*f_pos >= dev->size)
        goto out;
    if (*f_pos + count > dev->size)
        count = dev->size - *f_pos;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (dptr == NULL || !dptr->data || !dptr->data[s_pos])
        goto out;

    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_to_user_nofault(buf, dptr->data[s_pos] + q_pos, count))
    {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    out:
        mutex_unlock(&dev->mutex);
        return retval;
}

ssize_t scull_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct scull_dev *dev = filep->private_data;
    struct scull_qset *dptr;

    int quantum = dev->quantum, qset = dev->qset;
    int itemsize = quantum * qset; // Amount of bytes in the list
    int item, s_pos, q_pos, rest;
    ssize_t retval = -ENOMEM;

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

    item = (long)*f_pos / itemsize;
    rest = (long)*f_pos % itemsize;
    s_pos = rest / quantum;
    q_pos = rest % quantum;

    dptr = scull_follow(dev, item);
    if (dptr == NULL)
        goto out;
    if (!dptr->data)
    {
        dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
        if (!dptr->data)
            goto out;
        memset(dptr->data, 0, qset * sizeof(char *));
    }
    if (!dptr->data[s_pos])
    {
        dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
        if (!dptr->data)
            goto out;
    }

    if (count > quantum - q_pos)
        count = quantum - q_pos;

    if (copy_from_user_nmi(dptr->data[s_pos] + q_pos, buf, count))
    {
        retval = -EFAULT;
        goto out;
    }

    *f_pos += count;
    retval = count;

    out:
        mutex_unlock(&dev->mutex);
        return retval;
}

int scull_open(struct inode *inode, struct file *filep)
{
    struct scull_dev *dev;

    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filep->private_data = dev;

    if ((filep->f_flags & O_ACCMODE) == O_WRONLY)
    {
        scull_trim(dev);
    }

    return 0;
}

int scull_release(struct inode *inode, struct file *filep)
{
    return 0l;
}

static void scull_setup_cdev(struct scull_dev *dev, int index)
{
    int err, devno = MKDEV(scull_major, scull_minor + index);
    cdev_init(&dev->cdev, &scull_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &scull_fops;
    err = cdev_add(&dev->cdev, devno, 1);

    if (err)
    {
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
    //unregister_chrdev_region(, scull_nr_devs);
}

module_init(initialization);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple Character Utility for Loading Localities")
MODULE_AUTHOR("Kosta S.");