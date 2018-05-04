#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>

MODULE_LICENSE("Dual BSD/GPL");

/**
 * Init/De init area
 */

static int __init init_module(void)
{
    printk(KERN_ALERT "Hello, world\n");
    return 0;
}

static void __exit exit_module(void)
{
    printk(KERN_ALERT "Goodbye, cruel world\n");
}

module_init(init_module);
module_exit(exit_module);
