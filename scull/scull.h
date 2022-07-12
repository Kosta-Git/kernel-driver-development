#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h>

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0 // Dynamic major by default
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4 // scull0 through scull3
#endif

extern int scull_major;
extern int scull_nr_devs;

loff_t scull_llseek(struct file *filep, loff_t off, int whence);
ssize_t scull_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos);
ssize_t scull_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos);
int scull_open(struct inode* inode, struct file* filep);
int scull_release(struct inode* inode, struct file* filep);

struct scull_qset {
	void **data;
	struct scull_qset *next;
};

struct scull_dev {
    struct scull_qset* data; // Pointer to first quantum set
    int quantum;             // Current quantum size
    int qset;                // Current array size
    unsigned long size;      // Amount of data stored here
    unsigned int access_key  // Used by sculluid and scullpriv
    struct semaphore sem;    // Mutual exclusion semaphore
    struct cdev cdev;        // Char device
}

#endif /* _SCULL_H_ */