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
static int fiber_proc_show(struct seq_file *sfile, void *v);

// clang-format off

static struct file_operations fiber_proc_file_ops = {
    .owner = THIS_MODULE,
    .open = fiber_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
// clang-format on

/*
 * Implementations
 */

/**
 * @brief Init here all the proc files for the module
 * Inspired by https://static.lwn.net/images/pdf/LDD3/ch04.pdf and updated to new standard
 * @return int
 */
int init_proc() {
    struct proc_dir_entry *entry;
    entry = proc_create(PROC_ENTRY, 0, NULL, &fiber_proc_file_ops);
    if (!entry) {
        printk(KERN_ALERT MODULE_NAME PROC_LOG "registering /proc/" PROC_ENTRY " failed");
        return -1;
    }
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "registering /proc/" PROC_ENTRY " success");
    return 0;
}

/**
 * @brief Destroy all the proc files
 *
 */
void destroy_proc() {
    remove_proc_entry(PROC_ENTRY, NULL);
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "/proc/" PROC_ENTRY " destroyed");
}

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
    return single_open(file, fiber_proc_show, NULL);
}
/**
 * @brief Show the true data at itearator position
 *
 * @param sfile
 * @param v
 * @return int
 */
static int fiber_proc_show(struct seq_file *sfile, void *v) {
    seq_printf(sfile, "We have n processes!\n");
    return 0;
}