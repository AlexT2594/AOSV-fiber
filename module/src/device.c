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
 * @brief This file contains the implementation of the char device
 *
 * @file device.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-06
 */
#include "device.h"

/*
 * Static functions
 */
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long fiber_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

/*
 * Variables
 */

static fiber_dev_t fiber_dev;
static int is_device_open = 0;
// commands
char *cmds[FIBER_IOC_MAXNR + 1] = {
    "RESET",                   // 0
    "CONVERT_THREAD_TO_FIBER", // 1
    "CREATE_FIBER",            // 2
    "SWITCH_TO_FIBER",         // 3
    "FLS_ALLOC",               // 4
    "FLS_FREE",                // 5
    "FLS_GET",                 // 6
    "FLS_SET",                 // 7
    "EXIT"                     // 8
};

// clang-format off
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = fiber_ioctl,
    .compat_ioctl = fiber_ioctl
};
// clang-format on

/**
 * @brief Init the device
 * inspired by https://www.linuxjournal.com/article/2920
 * @return int
 */
int init_device(void) {
    int ret = 0;
    fiber_dev.device.minor = MISC_DYNAMIC_MINOR;
    fiber_dev.device.name = FIBER_DEVICE_NAME;
    fiber_dev.device.mode = 0666;
    fiber_dev.device.fops = &fops;
    fiber_dev.device.mode = S_IALLUGO;
    // register the device
    ret = misc_register(&(fiber_dev.device));
    if (ret < 0) {
        printk(KERN_ALERT MODULE_NAME DEVICE_LOG "/dev/" FIBER_DEVICE_NAME " creation error");
        return -1;
    }
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "/dev/" FIBER_DEVICE_NAME " successfully created");
    return SUCCESS;
}

/**
 * @brief Destroy the device
 *
 */
void destroy_device(void) {
    // deregister the device
    misc_deregister(&(fiber_dev.device));
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "/dev/" FIBER_DEVICE_NAME " successfully destroyed");
}

/*
 * Implementation of static functions
 */

static DEFINE_MUTEX(fiber_lock);

/**
 * @brief The main ioctl function, dispatcher of all the exposed capabilities of the module - aka
 * syscalls.
 * Inspired by https://static.lwn.net/images/pdf/LDD3/ch06.pdf
 *
 * @param inode
 * @param filp
 * @param cmd command number
 * @param arg possible pointer to a data structure in user space
 */
static long fiber_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int err = 0, retval = 0;
    mutex_lock(&fiber_lock);
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "IOCTL %s from pid %d, tgid %d", cmds[_IOC_NR(cmd)],
           current->pid, current->tgid);
    // check correctness of type and command number
    if (_IOC_TYPE(cmd) != FIBER_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > FIBER_IOC_MAXNR) return -ENOTTY;
    // check addresses before performing operations
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;

    printk(KERN_CONT "...mem ok!");

    switch (cmd) {
    case FIBER_IOCRESET:
        break;
    case FIBER_IOC_CONVERTTHREADTOFIBER:
        retval = convert_thread_to_fiber();
        break;
    case FIBER_IOC_CREATEFIBER:
        retval = create_fiber((fiber_params_t *)arg);
        break;
    case FIBER_IOC_SWITCHTOFIBER:
        retval = switch_to_fiber((unsigned)arg);
        break;
    case FIBER_IOC_FLS_ALLOC:
        retval = fls_alloc();
        break;
    case FIBER_IOC_FLS_FREE:
        retval = fls_free((long)arg);
        break;
    case FIBER_IOC_FLS_GET:
        retval = fls_get((fls_params_t *)arg);
        break;
    case FIBER_IOC_FLS_SET:
        retval = fls_set((fls_params_t *)arg);
        break;
    case FIBER_IOC_EXIT:
        retval = exit_fibered();
    default:
        break;
    }
    mutex_unlock(&fiber_lock);
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "IOCTL %s from pid %d, tgid %d => Retvalue: %d",
           cmds[_IOC_NR(cmd)], current->pid, current->tgid, retval);
    return retval;
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *filp) {
    // if (is_device_open) return -EBUSY;
    is_device_open++;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *filp) {
    is_device_open--;
    module_put(THIS_MODULE);
    return SUCCESS;
}

/*
 * Called when a process, which already opened the dev file, attempts to read
 * from it.
 */
static ssize_t device_read(struct file *filp, /* see include/linux/fs.h   */
                           char *buffer,      /* buffer to fill with data */
                           size_t length,     /* length of the buffer     */
                           loff_t *offset) {
    return 0;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off) {
    return 0;
}

/*
 * Utils
 */

/**
 * @brief Force close the device descriptor opened by the current process
 *
 * # Implementation
 * The function searches in the file descriptor array of the current process if there is a file
 * descriptor that is associated with a file object whose inode has as real device `i_rdev` a device
 * with the same minor of our device. Since our device is a `miscdevice` the minor number of its
 * `dev_t` will identify it uniquely in the system
 *
 * @return int The result of the called `sys_close` on that file descriptor
 */
int close_device_descriptor() {
    struct file **fd_array;
    struct fdtable *fd_table;
    unsigned fd = 0;
    int ret = 0;

    fd_table = files_fdtable(current->files);
    fd_array = fd_table->fd;
    for (fd = 0; fd < fd_table->max_fds; fd++)
        // check for an open fd with the same minor of the fiber device and if found close it
        if (fd_is_open(fd, fd_table) &&
            MAJOR(fiber_dev.device.this_device->devt) == MAJOR(fd_array[fd]->f_inode->i_rdev) &&
            fiber_dev.device.minor == MINOR(fd_array[fd]->f_inode->i_rdev)) {
            ret = sys_close(fd);
            break;
        }

    if (ret < 0) printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "Error while closing the fd");
    return ret;
}