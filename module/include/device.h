/*
 * Originally from https://gist.github.com/brenns10/65d1ee6bb8419f96d2ae693eb7a66cc0
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.h"
#include "ioctlcmd.h"
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

#define DEVICE_LOG ": DEV: "

int init_device(void);
void destroy_device(void);

#define FIBER_CLASS_NAME "fbrdrv"
#define FIBER_DEVICE_NAME "fiber"
#define FIBER_DEVICE_MINOR 0
#define BUF_LEN 80

/**
 * @brief The main structure of the fiber char device
 *
 */
typedef struct fiber_dev {
    dev_t dev;
    struct class *dev_class;
    struct device *device;
    struct cdev *cdev;
    struct semaphore sem;
} fiber_dev_t;

#endif