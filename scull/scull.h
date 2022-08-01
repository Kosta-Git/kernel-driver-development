#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h>
#include <linux/mutex.h>

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0 // Dynamic major by default
#endif

#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4 // scull0 through scull3
#endif

/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "scull_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of SCULL_QUANTUM bytes.
 *
 * The array (quantum-set) is SCULL_QSET long.
 */
#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET    1000
#endif

/*
 * The pipe device is a simple circular buffer. Here its default size
 */
#ifndef SCULL_P_BUFFER
#define SCULL_P_BUFFER 4096
#endif

extern int scull_major;
extern int scull_nr_devs;
extern int scull_quantum;
extern int scull_qset;

int scull_trim(struct scull_dev *dev);
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
    unsigned int access_key; // Used by sculluid and scullpriv
    struct mutex mutex;      // Mutual exclusion muxtex
    struct cdev cdev;        // Char device
}

#endif /* _SCULL_H_ */