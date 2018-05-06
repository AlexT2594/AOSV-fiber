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

/*
 * Global variables withing the file
 */
static int major;
static int is_device_open = 0;
static char msg[BUF_LEN];
static char *msg_ptr;

// clang-format off
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};
// clang-format on

/**
 * Init the device
 */
int init_device(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        printk(KERN_ALERT MODULE_NAME ": Registering char device " DEVICE_NAME " failed with %d\n", major);
        return major;
    }

    printk(KERN_DEBUG MODULE_NAME ": device " DEVICE_NAME " created with maj %d", major);

    // printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    // printk(KERN_INFO "the driver, create a dev file with\n");
    // printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major);
    // printk(KERN_INFO "Try various minor numbers. Try to cat and echo to\n");
    // printk(KERN_INFO "the device file.\n");
    // printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/**
 * Destroy the device
 */
void destroy_device(void) { unregister_chrdev(major, DEVICE_NAME); }

/*
 * Methods
 */

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