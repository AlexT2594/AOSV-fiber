/**
 * @brief This file is the starting point of the module
 *
 * @file fiber.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-06
 */
#include "fiber.h"

MODULE_LICENSE("Dual BSD/GPL");

/**
 * @brief The init function of the module
 * inspired by https://static.lwn.net/images/pdf/LDD3/ch02.pdf
 * @return int init_fiber
 */
static int __init init_fiber(void) {
    int err = 0;
    // init the char device
    printk(KERN_INFO MODULE_NAME FIBER_LOG "loading module\n");
    err |= init_device();
    if (err) goto device_err;
    err |= init_proc();
    if (err) goto proc_err;
    printk(KERN_INFO MODULE_NAME FIBER_LOG "loaded successfully\n");
    return SUCCESS;
proc_err:
    destroy_device();
device_err:
    printk(KERN_ALERT MODULE_NAME FIBER_LOG "loading error\n");
    return -1;
}

static void __exit destroy_fiber(void) {
    // destroy the char device
    destroy_device();
    destroy_proc();
    printk(KERN_INFO MODULE_NAME FIBER_LOG "unloaded successfully\n");
}

module_init(init_fiber);
module_exit(destroy_fiber);
