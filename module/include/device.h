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