// Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
// <alex.tufa94@gmail.com>
//
// This file is part of Fibers (Kernel Module).
//
// Fibers (Kernel Module) is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fibers (Kernel Module) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Fibers (Kernel Module).  If not, see <http://www.gnu.org/licenses/>.
//

/**
 * @brief This file contains definitions and macros for the *proc* part of the module
 *
 * @file proc.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#ifndef __PROC_H
#define __PROC_H

#include "common.h"
#include "core.h"
#include <asm/sections.h>
#include <linux/kallsyms.h>
#include <linux/mm.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_LOG ": PROC: "

/*
 * Exposed methods
 */

int init_proc(void);
void destroy_proc(void);

int replace_proc_pident_lookup(void);
void test_fn(void);

inline int is_kernel_inittext(unsigned long addr);
inline int is_kernel_text(unsigned long addr);
inline int is_kernel(unsigned long addr);
int is_ksym_addr(unsigned long addr);
unsigned long kallsyms_sym_address(int *kallsyms_offsets, unsigned long kallsyms_relative_base,
                                   int idx);
int kallsyms_reverse_addr(unsigned long kallsyms_relative_base, unsigned long addr_to_reverse);

#define PROC_FOLDER "fibers"
#define PROC_ENTRY "fiber"

/*
 * Needed structs
 */
union proc_op {
    int (*proc_get_link)(struct dentry *, struct path *);
    int (*proc_show)(struct seq_file *m, struct pid_namespace *ns, struct pid *pid,
                     struct task_struct *task);
};

struct pid_entry {
    const char *name;
    unsigned int len;
    umode_t mode;
    const struct inode_operations *iop;
    const struct file_operations *fop;
    union proc_op op;
};

#endif