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

// sections values
unsigned long ___sinittext;
unsigned long ___einittext;
unsigned long ___stext;
unsigned long ___end;
unsigned long ___stext;
unsigned long ___etext;
unsigned long ___sdata;

unsigned long cr0;

// backup value
typedef asmlinkage struct dentry *(*original_proc_pident_lookup_t)(struct inode *, struct dentry *,
                                                                   const struct pid_entry *,
                                                                   unsigned int);
original_proc_pident_lookup_t original_proc_pident_lookup;

unsigned long original_proc_pident_lookup_pos; // position inside the table
int original_proc_pident_lookup_offset;        // value in the kallsyms_offsets table
int *kallsyms_offsets;                         // to find

static inline void protect_memory(void) { write_cr0(cr0); }
static inline void unprotect_memory(void) { write_cr0(cr0 & ~0x00010000); }

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
    int ret = 0;
    // save cr0
    cr0 = read_cr0();

    entry = proc_create(PROC_ENTRY, 0, NULL, &fiber_proc_file_ops);
    if (!entry) {
        printk(KERN_ALERT MODULE_NAME PROC_LOG "registering /proc/" PROC_ENTRY " failed");
        return -1;
    }

    // get values of sections delimiters
    ___einittext = kallsyms_lookup_name("_einittext");
    ___sinittext = kallsyms_lookup_name("_sinittext");
    ___stext = kallsyms_lookup_name("_stext");
    ___end = kallsyms_lookup_name("_end");
    ___stext = kallsyms_lookup_name("_stext");
    ___etext = kallsyms_lookup_name("_etext");
    ___sdata = kallsyms_lookup_name("_sdata");

    ret = replace_proc_pident_lookup();
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "registering /proc/" PROC_ENTRY " success");
    return 0;
}

/**
 * @brief Destroy all the proc files
 *
 */
void destroy_proc() {
    remove_proc_entry(PROC_ENTRY, NULL);
    unprotect_memory();
    kallsyms_offsets[original_proc_pident_lookup_pos] = original_proc_pident_lookup_offset;
    protect_memory();
    printk(KERN_DEBUG MODULE_NAME PROC_LOG "/proc/" PROC_ENTRY " destroyed");
}

/*
 * Kallsyms hacking part
 */

struct dentry *fiber_proc_pident_lookup(struct inode *dir, struct dentry *dentry,
                                        const struct pid_entry *ents, unsigned int nents) {
    struct dentry *res = original_proc_pident_lookup(dir, dentry, ents, nents);
    printk(KERN_DEBUG MODULE_NAME PROC_LOG
           "fiber_proc_pident_lookup(). Hacking success, you're great");
    return res;
}

int replace_proc_pident_lookup() {
    int ret = 0, reversed_off;
    unsigned long off;
    // get addresses
    unsigned long proc_pidentry_lookup_addr = kallsyms_lookup_name("proc_pident_lookup");
    unsigned long get_symbol_pos_addr = kallsyms_lookup_name("get_symbol_pos");
    // we can see that proc_pident_lookup is near to the inat_primary_table declaration
    unsigned long inat_primary_table_addr = kallsyms_lookup_name("inat_primary_table");
    // we use this function to retrieve the number of proc_pident_lookup in the kallsyms table
    unsigned long (*get_symbol_pos)(unsigned long, unsigned long, unsigned long) =
        (unsigned long (*)(unsigned long, unsigned long, unsigned long))get_symbol_pos_addr;
    // the pos in the table is the following
    original_proc_pident_lookup_pos = get_symbol_pos(proc_pidentry_lookup_addr, 0, 0);
    // our desire
    kallsyms_offsets = 0;

    // backup the value
    original_proc_pident_lookup = (original_proc_pident_lookup_t)proc_pidentry_lookup_addr;

    printk(KERN_DEBUG "proc_pidentry_lookup_addr = %#lx", proc_pidentry_lookup_addr);
    printk(KERN_DEBUG "original_proc_pident_lookup_pos = %lu", original_proc_pident_lookup_pos);
    printk(KERN_DEBUG "inat_primary_table_addr = %#lx", inat_primary_table_addr);
    printk(KERN_DEBUG "&fiber_proc_pident_lookup = %#lx", (unsigned long)&fiber_proc_pident_lookup);

    for (off = inat_primary_table_addr; off < ULONG_MAX; off += sizeof(void *)) {
        if (!is_ksym_addr(off)) continue;
        kallsyms_offsets = (int *)off;

        if (kallsyms_sym_address(kallsyms_offsets, ___stext, original_proc_pident_lookup_pos) ==
            proc_pidentry_lookup_addr) {
            printk("FOUND!! %#lx", off);
            break;
        }
    }

    // backup the offset
    original_proc_pident_lookup_offset = kallsyms_offsets[original_proc_pident_lookup_pos];
    // compute the reverse
    reversed_off = kallsyms_reverse_addr(___stext, (unsigned long)&fiber_proc_pident_lookup);

    printk(KERN_DEBUG "kallsyms_offsets = %#lx", (unsigned long)kallsyms_offsets);
    printk(KERN_DEBUG "reversed off is %d", reversed_off);
    printk(KERN_DEBUG "indeed base - 1 - off = %#lx (should &fiber_proc_pident_lookup)",
           ___stext - 1 - reversed_off);

    // modify original entry
    unprotect_memory();
    kallsyms_offsets[original_proc_pident_lookup_pos] =
        kallsyms_reverse_addr(___stext, (unsigned long)&fiber_proc_pident_lookup);
    protect_memory();

    proc_pidentry_lookup_addr = kallsyms_lookup_name("proc_pident_lookup");
    printk(KERN_DEBUG "Now proc_pidentry_lookup_addr = %#lx", proc_pidentry_lookup_addr);

    return ret;
}

void test_fn() {}

/*
 * Kallsyms utils area
 */
inline int is_kernel_inittext(unsigned long addr) {
    if (addr >= ___sinittext && addr <= ___einittext) return 1;
    return 0;
}

inline int is_kernel_text(unsigned long addr) {
    if ((addr >= ___stext && addr <= ___etext) || arch_is_kernel_text(addr)) return 1;
    // return in_gate_area_no_mm(addr);
    return 0;
}

/*
static inline int is_kernel_data(unsigned long addr) {
    unsigned long _sdata = kallsyms_lookup_name("_sdata");
    unsigned long _edata = kallsyms_lookup_name("_edata");
    if ((addr >= _sdata && addr <= _edata) || arch_is_kernel_data(addr)) return 1;
    return in_gate_area_no_mm(addr);
}
*/

inline int is_kernel(unsigned long addr) {
    if (addr >= ___stext && addr <= ___end) return 1;
    // return in_gate_area_no_mm(addr);
    return 0;
}

int is_ksym_addr(unsigned long addr) {
    if (IS_ENABLED(CONFIG_KALLSYMS_ALL)) return is_kernel(addr);
    return is_kernel_text(addr) || is_kernel_inittext(addr);
}

unsigned long kallsyms_sym_address(int *kallsyms_offsets, unsigned long kallsyms_relative_base,
                                   int idx) {
    /* ...otherwise, positive offsets are absolute values */
    if (kallsyms_offsets[idx] >= 0) return kallsyms_offsets[idx];

    /* ...and negative offsets are relative to kallsyms_relative_base - 1 */
    return kallsyms_relative_base - 1 - kallsyms_offsets[idx];
}

/**
 * @brief Compute the address to put in kallsyms_offsets[idx]
 *
 * @param kallsyms_relative_base
 * @param addr_to_reverse
 * @return int
 */
int kallsyms_reverse_addr(unsigned long kallsyms_relative_base, unsigned long addr_to_reverse) {
    if (addr_to_reverse > INT_MAX)
        return kallsyms_relative_base - 1 - addr_to_reverse;
    else
        return addr_to_reverse;
}

/*
 * Implementation of static functions for reading the proc files
 */

/**
 * @brief Implement here the method that register the operations of the file. In this case we
 * implement the proc file as it was a char device, so we have to define all the operations that
 * the fs should do
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
