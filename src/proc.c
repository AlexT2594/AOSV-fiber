/**
 * @brief This file contains the implementation of the /proc fs files
 *
 * @file proc.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-06
 */
#include "proc.h"

/*
 * Static declarations
 */
static int fiber_proc_open(struct inode *inode, struct file *file);
static void *fiber_proc_start(struct seq_file *sfile, loff_t *pos);
static void *fiber_proc_next(struct seq_file *sfile, void *v, loff_t *pos);
static void fiber_proc_stop(struct seq_file *sfile, void *v);
static int fiber_proc_show(struct seq_file *sfile, void *v);

// clang-format off
static struct seq_operations fiber_proc_ops = {
    .start = fiber_proc_start,
    .next = fiber_proc_next,
    .stop = fiber_proc_stop,
    .show = fiber_proc_show
};

static struct file_operations fiber_proc_file_ops = {
    .owner = THIS_MODULE,
    .open = fiber_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
// clang-format on

/*
 * Exposed functions
 */

/**
 * @brief Init here all the proc files for the module
 *
 * @return int
 */
int init_proc() {
    struct proc_dir_entry *entry;
    entry = proc_create(PROC_ENTRY, 0, NULL, &fiber_proc_file_ops);
    if (!entry) {
        printk(KERN_ALERT MODULE_NAME ": Registering /proc/" PROC_ENTRY " failed");
        return -1;
    }
    return 0;
}

/**
 * @brief Destroy all the proc files
 *
 */
void destroy_proc() { remove_proc_entry(PROC_ENTRY, NULL); }

/*
 * Implementation of static functions
 */

/**
 * @brief Implement here the method that register the operations of the file. In this case we
 * implement the proc file as it was a char device, so we have to define all the operations that the
 * fs should do
 *
 * @param inode
 * @param file
 * @return int
 */
static int fiber_proc_open(struct inode *inode, struct file *file) {
    return seq_open(file, &fiber_proc_ops);
}

/**
 * @brief Start of the iterator, we may want here to acquire semaphores/spinlocks
 *
 * @param sfile
 * @param pos
 * @return void*
 */
static void *fiber_proc_start(struct seq_file *sfile, loff_t *pos) {
    // let`s say we have only a single item to show
    if (*pos > 0) return NULL;
    return pos;
}

/**
 * @brief Get the next element in the iterator
 *
 * @param sfile
 * @param v
 * @param pos
 * @return void*
 */
static void *fiber_proc_next(struct seq_file *s, void *v, loff_t *pos) {
    loff_t *spos = v;
    *pos = ++*spos;
    if (*pos > 0) return NULL;
    return spos;
}

/**
 * @brief Clean after iterator ending
 *
 * @param sfile
 * @param v
 */
static void fiber_proc_stop(struct seq_file *sfile, void *v) {}

/**
 * @brief Show the true data at iteartor position
 *
 * @param sfile
 * @param v
 * @return int
 */
static int fiber_proc_show(struct seq_file *sfile, void *v) {
    seq_printf(sfile, "This proc file is working very well!\n");
    return 0;
}