#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>

static int hello_init(void)
{
    printk(KERN_ALERT "Hello, world\n");
    printk(KERN_ALERT "The process is \"%s\" (pid %i)\n", current->comm, current->pid);
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, world\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kosta S.");
MODULE_DESCRIPTION("Hello world module using Makefile");