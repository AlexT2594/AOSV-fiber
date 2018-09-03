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

typedef asmlinkage struct dentry *(*original_proc_pident_lookup_t)(struct inode *, struct dentry *,
                                                                   const struct pid_entry *,
                                                                   unsigned int);

typedef asmlinkage int (*original_proc_pident_readdir_t)(struct file *, struct dir_context *,
                                                         const struct pid_entry *, unsigned int);

typedef asmlinkage int (*original_pid_getattr_t)(const struct path *, struct kstat *, u32,
                                                 unsigned int);
typedef asmlinkage int (*original_proc_setattr_t)(struct dentry *, struct iattr *);

original_proc_pident_lookup_t original_proc_pident_lookup;
original_proc_pident_readdir_t original_proc_pident_readdir;
original_pid_getattr_t original_pid_getattr;
original_proc_setattr_t original_proc_setattr;
/*
 * Static declarations
 */
static struct dentry *fiber_proc_pident_lookup(struct inode *dir, struct dentry *dentry,
                                               const struct pid_entry *ents, unsigned int nents);
static int fiber_proc_pident_readdir(struct file *file, struct dir_context *ctx,
                                     const struct pid_entry *ents, unsigned int nents);

/**
 * Fibers FS
 */

static int fiber_proc_open(struct inode *inode, struct file *file);
static int fiber_proc_show(struct seq_file *sfile, void *v);
static struct dentry *proc_fibers_dir_lookup(struct inode *dir, struct dentry *dentry,
                                             unsigned int flags);
static int proc_fibers_dir_readdir(struct file *file, struct dir_context *ctx);

// clang-format off
static struct inode_operations proc_fibers_folder_inode_operations; /* = {
    .lookup = proc_fibers_dir_lookup,
    .getattr = original_pid_getattr,
    .setattr = original_proc_setattr,
    //.permission = proc_pid_permission,
}; */

static const struct file_operations proc_fibers_folder_operations = {
    .read = generic_read_dir,
    .iterate_shared = proc_fibers_dir_readdir, 
    .llseek = generic_file_llseek
};

static struct file_operations fiber_proc_file_ops = {
    .owner = THIS_MODULE,
    .open = fiber_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = seq_release
};
// clang-format on

static const struct pid_entry fibers_dir_stuff[] = {
    REG("hello1", S_IRUGO | S_IWUGO, fiber_proc_file_ops),
    REG("hello2", S_IRUGO | S_IWUGO, fiber_proc_file_ops)};

static struct ftrace_hook hooked_functions[] = {
    HOOK("proc_pident_readdir", fiber_proc_pident_readdir, &original_proc_pident_readdir),
    // HOOK("proc_pident_lookup", fiber_proc_pident_lookup, &original_proc_pident_lookup),
};

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
    original_proc_pident_lookup =
        (original_proc_pident_lookup_t)kallsyms_lookup_name("proc_pident_lookup");
    original_proc_setattr = (original_proc_setattr_t)kallsyms_lookup_name("proc_setattr");
    original_pid_getattr = (original_pid_getattr_t)kallsyms_lookup_name("pid_getattr");
    proc_fibers_folder_inode_operations.lookup = proc_fibers_dir_lookup;
    proc_fibers_folder_inode_operations.getattr = original_pid_getattr;
    proc_fibers_folder_inode_operations.setattr = original_proc_setattr;
    fh_install_hook(&hooked_functions[0]);
    return 0;
}

/**
 * @brief Destroy all the proc files
 *
 */
void destroy_proc() {
    remove_proc_entry(PROC_ENTRY, NULL);
    fh_remove_hook(&hooked_functions[0]);
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "/proc/" PROC_ENTRY " destroyed");
}

static int fiber_proc_pident_readdir(struct file *file, struct dir_context *ctx,
                                     const struct pid_entry *ents, unsigned int nents) {
    int res;
    struct pid_entry *ents_copy;
    // our dir
    struct pid_entry fibers_dir =
        DIR(PROC_FOLDER, S_IRUGO | S_IXUGO, proc_fibers_folder_inode_operations,
            proc_fibers_folder_operations);
    // check if we are not in /proc
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "Dir is %s", file->f_path.dentry->d_iname);
    // copy the ents array
    ents_copy = kzalloc((nents + 1) * sizeof(struct pid_entry), GFP_KERNEL);
    memcpy(ents_copy, ents, nents * sizeof(struct pid_entry));
    // add our entry
    memcpy(&ents_copy[nents], &fibers_dir, sizeof(struct pid_entry));
    res = original_proc_pident_readdir(file, ctx, ents_copy, nents + 1);
    kfree(ents_copy);
    return res;
}

static struct dentry *fiber_proc_pident_lookup(struct inode *dir, struct dentry *dentry,
                                               const struct pid_entry *ents, unsigned int nents) {
    struct dentry *res;
    struct pid_entry *ents_copy;
    // our dir
    struct pid_entry fibers_dir =
        DIR(PROC_FOLDER, S_IRUGO | S_IXUGO, proc_fibers_folder_inode_operations,
            proc_fibers_folder_operations);
    // check if we are not in /proc
    printk(
        KERN_DEBUG MODULE_NAME PROC_LOG "Dir is %s",
        ((struct dentry *)container_of(dir->i_dentry.first, struct dentry, d_u.d_alias))->d_iname);
    // copy the ents array
    ents_copy = kzalloc((nents + 1) * sizeof(struct pid_entry), GFP_KERNEL);
    memcpy(ents_copy, ents, nents * sizeof(struct pid_entry));
    // add our entry
    memcpy(&ents_copy[nents], &fibers_dir, sizeof(struct pid_entry));
    res = original_proc_pident_lookup(dir, dentry, ents_copy, nents + 1);
    kfree(ents_copy);
    return res;
}

/*
 * Ftrace area
 */

static void notrace fh_ftrace_thunk(unsigned long ip, unsigned long parent_ip,
                                    struct ftrace_ops *ops, struct pt_regs *regs) {
    struct ftrace_hook *hook = container_of(ops, struct ftrace_hook, ops);
    /* Skip the function calls from the current module. */
    if (!within_module(parent_ip, THIS_MODULE)) regs->ip = (unsigned long)hook->function;
}

static int resolve_hook_address(struct ftrace_hook *hook) {
    hook->address = kallsyms_lookup_name(hook->name);
    if (!hook->address) {
        pr_debug("unresolved symbol: %s\n", hook->name);
        return -ENOENT;
    }
    *((unsigned long *)hook->original) = hook->address;
    return 0;
}

int fh_install_hook(struct ftrace_hook *hook) {
    int err;
    err = resolve_hook_address(hook);
    if (err) return err;

    hook->ops.func = fh_ftrace_thunk;
    hook->ops.flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_IPMODIFY;

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 0, 0);
    if (err) {
        pr_debug("ftrace_set_filter_ip() failed: %d\n", err);
        return err;
    }

    err = register_ftrace_function(&hook->ops);
    if (err) {
        pr_debug("register_ftrace_function() failed: %d\n", err);

        /* Don’t forget to turn off ftrace in case of an error. */
        ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);

        return err;
    }

    return 0;
}

void fh_remove_hook(struct ftrace_hook *hook) {
    int err;

    err = unregister_ftrace_function(&hook->ops);
    if (err) pr_debug("unregister_ftrace_function() failed: %d\n", err);

    err = ftrace_set_filter_ip(&hook->ops, hook->address, 1, 0);
    if (err) { pr_debug("ftrace_set_filter_ip() failed: %d\n", err); }
}

/*
 * Implementation of static functions
 */

/**
 * @brief Implements the assignment to every entry in dir /proc/<PID>/fibers
 *
 * @param dir
 * @param dentry
 * @param flags
 * @return struct dentry*
 */
static struct dentry *proc_fibers_dir_lookup(struct inode *dir, struct dentry *dentry,
                                             unsigned int flags) {
    return original_proc_pident_lookup(dir, dentry, fibers_dir_stuff, ARRAY_SIZE(fibers_dir_stuff));
}

/**
 * @brief Implements the reading of the /proc/<PID>/fibers dir
 *
 * @param file
 * @param ctx
 * @return int
 */
static int proc_fibers_dir_readdir(struct file *file, struct dir_context *ctx) {
    return original_proc_pident_readdir(file, ctx, fibers_dir_stuff, ARRAY_SIZE(fibers_dir_stuff));
}

/**
 * @brief Implement here the method that register the operations of the file. In this case we
 * implement the proc file as it was a char device, so we have to define all the operations that
 * the
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