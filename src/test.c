#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("Dual BSD/GPL");

static int __init initializer(void)
{
    printk(KERN_ALERT "Hello, world\n");
    return 0;
}

static void __exit exitizer(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(initializer);
module_exit(exitizer);
