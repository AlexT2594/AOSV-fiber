/**
 * Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
 * <alex.tufa94@gmail.com>
 *
 * This file is part of Fibers (Kernel Module).
 *
 * Fibers (Kernel Module) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fibers (Kernel Module) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fibers (Kernel Module).  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @brief This file is the starting point of the module
 *
 * @file fiber.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-06
 */
#include "fiber.h"

MODULE_LICENSE("GPL");

/**
 * @brief The init function of the module
 *
 * This is the entry point of the module. Every kind of default structures and services are
 * instantiated here.
 *
 * Inspired by https://static.lwn.net/images/pdf/LDD3/ch02.pdf
 *
 * @return int 0 if everything ok, otherwise -1
 */
static int __init init_fiber(void) {
    int err = 0;
    // init the char device
    printk(KERN_INFO MODULE_NAME FIBER_LOG "loading module\n");
    err |= init_device();
    if (err) goto proc_err;
    err |= init_core();
    if (err) goto device_err;
    err |= init_proc();
    if (err) goto core_err;
    printk(KERN_INFO MODULE_NAME FIBER_LOG "loaded successfully\n");
    return SUCCESS;
proc_err:
    destroy_proc();
core_err:
    destroy_core();
device_err:
    destroy_device();
    printk(KERN_ALERT MODULE_NAME FIBER_LOG "loading error\n");
    return -1;
}

/**
 * @brief The destroy function of the module
 *
 * This function deallocate all memory and deregister every service used
 */
static void __exit destroy_fiber(void) {
    destroy_proc();
    destroy_core();
    destroy_device();
    printk(KERN_INFO MODULE_NAME FIBER_LOG "unloaded successfully\n");
}

module_init(init_fiber);
module_exit(destroy_fiber);
