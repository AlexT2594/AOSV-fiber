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
static DECLARE_WAIT_QUEUE_HEAD(wq);

/*
 * Global variables withing the file
 */
static fiber_dev_t fiber_dev;
static int is_device_open = 0;
static char msg[BUF_LEN];
static char *msg_ptr;

// clang-format off
static fibered_processes_list_t fibered_processes_list = {
    .head = NULL,
    .tail = NULL,
    .processes_count = 0
};
static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = fiber_ioctl,
    .compat_ioctl = fiber_ioctl
};
// clang-format on

/*
 * Exposed functions
 */

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
    int err = 0;
    int retval = 0;
    struct pt_regs *regs = task_pt_regs(current);
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "Called IOCTL with cmd %d", _IOC_NR(cmd));

    // check correctness of type and command number
    if (_IOC_TYPE(cmd) != FIBER_IOC_MAGIC) return -ENOTTY;
    if (_IOC_NR(cmd) > FIBER_IOC_MAXNR) return -ENOTTY;
    // check addresses before performing operations
    if (_IOC_DIR(cmd) & _IOC_READ) err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    if (_IOC_DIR(cmd) & _IOC_WRITE) err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    if (err) return -EFAULT;

    switch (cmd) {
    case FIBER_IOCRESET:
        break;
    case FIBER_IOC_CONVERTTHREADTOFIBER:
        printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "Called FIBER_IOC_CONVERTTHREADTOFIBER from pid %d, tgid %d",
               current->pid, current->tgid);
        return convert_thread_to_fiber();
        break;
    case FIBER_IOC_CREATEFIBER:
        printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "Called FIBER_IOC_CREATEFIBER from pid %d, tgid %d", current->pid,
               current->tgid);
        printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "Passed arg %lu", arg);
        // set rip to desired function
        regs->ip = arg;
        return 0;
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

int convert_thread_to_fiber() {
    // check if process already created at least a fiber
    fibered_process_node_t *fibered_process_node = fibered_processes_list.head;
    fiber_node_t *fiber_node;
    uint8_t found = 0;
    while (fibered_process_node != NULL) {
        if (fibered_process_node->pid == current->tgid) {
            found = 1;
            break;
        }
        fibered_process_node = fibered_process_node->next;
    }
    // process has never created a fiber
    if (!found) {
        fibered_process_node = kmalloc(sizeof(fibered_process_node_t), GFP_KERNEL);
        fibered_process_node->pid = current->tgid;
        fibered_processes_list.processes_count = 1;
        fibered_process_node->prev = NULL;
        fibered_process_node->next = NULL;
        fibered_process_node->fibers_list = kmalloc(sizeof(fibers_list_t), GFP_KERNEL);
        fibered_process_node->fibers_list->fibers_count = 0;
        fibered_process_node->fibers_list->head = NULL;
        fibered_process_node->fibers_list->tail = NULL;
        // if head is null we have to init the list
        if (fibered_processes_list.head == NULL) fibered_processes_list.head = fibered_process_node;
        // otherwise we append to the list
        if (fibered_processes_list.tail != NULL) {
            fibered_processes_list.tail->next = fibered_process_node;
            fibered_process_node->prev = fibered_processes_list.tail;
        }
        fibered_processes_list.tail = fibered_process_node;
    }
    printk(KERN_DEBUG MODULE_NAME DEVICE_LOG "tgid is %d", fibered_process_node->pid);
    // check if thread is already a fiber
    fiber_node = fibered_process_node->fibers_list->head;
    found = 0;
    while (fiber_node != NULL) {
        if (fiber_node->created_by == current->pid) {
            found = 1;
            break;
        }
        fiber_node = fiber_node->next;
    }
    if (!found) {
        fiber_node = kmalloc(sizeof(fiber_node_t), GFP_KERNEL);
        fiber_node->prev = NULL;
        fiber_node->next = NULL;
        fiber_node->created_by = current->pid;
        fibered_process_node->fibers_list->fibers_count++;
        if (fibered_process_node->fibers_list->head == NULL) fibered_process_node->fibers_list->head = fiber_node;
        if (fibered_process_node->fibers_list->tail != NULL) {
            fibered_process_node->fibers_list->tail->next = fiber_node;
            fiber_node->prev = fibered_process_node->fibers_list->tail;
        }
        fibered_process_node->fibers_list->tail = fiber_node;
    } else
        return -THREAD_ALREADY_FIBER;

    fiber_node->id = fibered_process_node->fibers_list->fibers_count - 1;
    fiber_node->state = RUNNING;

    return 0;
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
static ssize_t device_write(struct file *filp, const char *buf, size_t len, loff_t *off) { return 0; }