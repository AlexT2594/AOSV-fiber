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
 * Global variables withing the file
 */
static fiber_dev_t fiber_dev;
static int major;
static int is_device_open = 0;
static char msg[BUF_LEN];
static char *msg_ptr;

// clang-format off
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = fiber_ioctl
};
// clang-format on

/*
 * Exposed functions
 */

/**
 * @brief Init the device
 *
 * @return int
 */
int init_device(void) {
    // request a major and a set of minors
    int res = alloc_chrdev_region(&(fiber_dev.dev), 0, 1, FIBER_DEVICE_NAME);
    major = MAJOR(fiber_dev.dev);

    if (res < 0) {
        printk(KERN_ALERT MODULE_NAME ": registering char region with major %d failed\n", major);
        return res;
    }
    printk(KERN_DEBUG MODULE_NAME ": device created with major %d", major);

    // register the character device cdev
    fiber_dev.cdev = cdev_alloc();
    cdev_init(fiber_dev.cdev, &fops);
    fiber_dev.cdev->owner = THIS_MODULE;
    res = cdev_add(fiber_dev.cdev, fiber_dev.dev, 1);
    if (res < 0) {
        printk(KERN_ALERT MODULE_NAME ": adding char device with major %d failed\n", major);
        return res;
    }
    printk(KERN_DEBUG MODULE_NAME ": char device added");

    // create the class and the inode
    fiber_dev.dev_class = class_create(THIS_MODULE, FIBER_DEVICE_NAME);
    fiber_dev.device =
        device_create(fiber_dev.dev_class, NULL, fiber_dev.dev, NULL, FIBER_DEVICE_NAME);
    if (!IS_ERR(fiber_dev.device)) {
        printk(KERN_ALERT MODULE_NAME ": /dev/" FIBER_DEVICE_NAME " creation error");
        return -1;
    }
    printk(KERN_DEBUG MODULE_NAME ": /dev/" FIBER_DEVICE_NAME " successfully created");

    return SUCCESS;
}

/**
 * @brief Destroy the device
 *
 */
void destroy_device(void) {
    // destroy device
    device_destroy(fiber_dev.dev_class, fiber_dev.dev);
    // destroy class
    class_destroy(fiber_dev.dev_class);
    // destroy the character device
    cdev_del(fiber_dev.cdev);
    // dealloc the major region
    unregister_chrdev_region(fiber_dev.dev, 1);
}

/*
 * Implementation of static functions
 */

/**
 * @brief The main ioctl function, dispatcher of all the exposed capabilities of the module - aka
 * syscalls
 *
 * @param inode
 * @param filp
 * @param cmd command number
 * @param arg possible pointer to a data structure in user space
 */
static long fiber_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int err = 0;
    int retval = 0;
    // check correctness of type and command number
    if (_IOC_TYPE(cmd) != FIBER_IOC_MAGIC) return -ENOTTY;
    if (_IOC_TYPE(cmd) > FIBER_IOC_MAXNR) return -ENOTTY;
    // check addresses before performing operations
    if (_IOC_DIR(cmd) & _IOC_READ)
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    if (_IOC_DIR(cmd) & _IOC_WRITE)
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;

    switch (cmd) {
    case FIBER_IOCRESET:
        break;
    case FIBER_IOC_CONVERTTOFIBER:
        break;
    case FIBER_IOC_CREATEFIBER:
        break;
    case FIBER_IOC_SWITCHTOFIBER:
        break;
    case FIBER_IOC_FLS_ALLOC:
        // call __get_user for getting the passed data structure
        break;
    case FIBER_IOC_FLS_FREE:
        // call __get_user for getting the passed data structure
        break;
    case FIBER_IOC_FLS_GET:
        // call __get_user for getting the passed data structure
        break;
    case FIBER_IOC_FLS_SET:
        // call __get_user for getting the passed data structure
        break;
    default:
        break;
    }
    return retval;
}

/*
 * Called when a process tries to open the device file, like
 * "cat /dev/mycharfile"
 */
static int device_open(struct inode *inode, struct file *filp) {
    static int counter = 0;

    if (is_device_open) return -EBUSY;

    is_device_open++;
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);
    msg_ptr = msg;
    /*
     * TODO: comment out the line below to have some fun!
     */
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *filp) {
    is_device_open--;

    /*
     * Decrement the usage count, or else once you opened the file, you'll never
     * get rid of the module.
     *
     * TODO: comment out the line below to have some fun!
     */
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
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_read = 0;

    /*
     * If we're at the end of the message, return 0 signifying end of file.
     */
    if (*msg_ptr == 0) return 0;

    /*
     * Actually put the data into the buffer
     */
    while (length && *msg_ptr) {
        /*
         * The buffer is in the user data segment, not the kernel segment so "*"
         * assignment won't work. We have to use put_user which copies data from the
         * kernel data segment to the user data segment.
         */
        // put_user(*(msg_ptr++), buffer++);
        length--;
        bytes_read++;
    }

    /*
     * Most read functions return the number of bytes put into the buffer
     */
    return bytes_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/hello
 */
static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off) {
    printk(KERN_ALERT "Sorry, this operation isn't supported.\n");
    return -EINVAL;
}