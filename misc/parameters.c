#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

static char* whom = "world";
static int how_many = 1;

module_param(how_many, int, S_IRUGO);
module_param(whom, charp, S_IRUGO);

static int __init initialization(void)
{
    printk(KERN_ALERT "%i * Hello, %s\n", how_many, whom);
    printk(KERN_ALERT "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
    return 0;
}

static void __exit cleanup(void)
{
    printk(KERN_ALERT "Goodbye, world\n");
}

module_init(initialization);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosta S.");