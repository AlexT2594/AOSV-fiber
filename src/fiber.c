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

static int __init init_fiber(void) {
    // init the char device
    printk(KERN_INFO MODULE_NAME ": loading module\n");
    int err = 0;
    err |= init_device();
    err |= init_proc();
    if (err) {
        printk(KERN_ALERT MODULE_NAME ": loaded successfully\n");
        return -1; // TODO what return here?
    }
    printk(KERN_INFO MODULE_NAME ": loaded successfully\n");
    return SUCCESS;
}

static void __exit destroy_fiber(void) {
    // destroy the char device
    destroy_device();
    destroy_proc();
    printk(KERN_INFO MODULE_NAME ": unloaded successfully\n");
}

module_init(init_fiber);
module_exit(destroy_fiber);
