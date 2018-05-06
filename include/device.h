/*
 * Originally from https://gist.github.com/brenns10/65d1ee6bb8419f96d2ae693eb7a66cc0
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.h"
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

int init_device(void);
void destroy_device(void);

#define SUCCESS 0
#define DEVICE_NAME "fiber"
#define BUF_LEN 80

/*
 * IOCTL Section
 * We define the following operations for the module
 * - CONVERTTOFIBER - ConvertThreadToFiber
 * - CREATEFIBER - CreateFiber
 * - SWITCHTOFIBER - SwitchToFiber
 * - FLS_ALLOC - FlsAlloc
 * - FLS_FREE - FlsFree
 * - FLS_GET - FlsGetValue
 * - FLS_SET - FlsSetValue
 */
#define FIBER_IOC_MAGIC 0xF1

#define FIBER_IOCRESET _IO(FIBER_IOC_MAGIC, 0)
#define FIBER_IOC_CONVERTTOFIBER _IO(FIBER_IOC_MAGIC, 1)
#define FIBER_IOC_CREATEFIBER _IO(FIBER_IOC_MAGIC, 2)
#define FIBER_IOC_SWITCHTOFIBER _IO(FIBER_IOC_MAGIC, 3)
#define FIBER_IOC_FLS_ALLOC _IOWR(FIBER_IOC_MAGIC, 4, int)
#define FIBER_IOC_FLS_FREE _IOW(FIBER_IOC_MAGIC, 5, int)
#define FIBER_IOC_FLS_GET _IOR(FIBER_IOC_MAGIC, 6, int)
#define FIBER_IOC_FLS_SET _IOW(FIBER_IOC_MAGIC, 7, int)

#define FIBER_IOC_MAXNR 7

/**
 * @brief The main structure of the fiber char device
 *
 */
struct fiber_dev {
    unsigned long fnum;
    struct semaphore sem;
    struct cdev cdev;
};

#endif