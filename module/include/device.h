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
 * @brief This file contains definitions and macros for the *device* of the module
 *
 * @file device.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.h"
#include "core.h"
#include "ioctlcmd.h"
#include <asm/current.h>
#include <asm/ptrace.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/sched/task_stack.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

#define DEVICE_LOG ": DEV: "

/*
 * Exposed methods
 */

int init_device(void);
void destroy_device(void);
// utils
int close_device_descriptor(void);

/*
 * Definitions
 */

#define FIBER_DEVICE_NAME "fiber"
#define BUF_LEN 80

/**
 * @brief The main structure of the fiber char device
 *
 */
typedef struct fiber_dev {
    struct miscdevice device;
} fiber_dev_t;

#endif