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
 * @brief The head of the core part of the module
 *
 * This file contains all the data structures and the methods that implements the core functions of
 * the fiber module
 *
 * @file core.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#ifndef __CORE_H
#define __CORE_H

#include "common.h"
#include "ioctlcmd.h"
#include "utils.h"

#include <asm/current.h>
#include <asm/fpu/internal.h>
#include <asm/fpu/types.h>
#include <asm/ptrace.h>
#include <linux/bitmap.h>
#include <linux/fdtable.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/sched/task_stack.h>
#include <linux/semaphore.h>
#include <linux/signal.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/uaccess.h>

#define CORE_LOG ": CORE: "

/*
 * Definitions
 */

typedef struct fibered_process fibered_process_node_t;
typedef struct fiber_processes_list fiber_process_list_t;
typedef struct fiber fiber_node_t;
typedef struct fibers_list fibers_list_t;
typedef struct fiber_params fiber_params_t;

/*
 * Exposed methods
 */

int init_core(void);
void destroy_core(void);
// implementations core fns
int convert_thread_to_fiber(void);
int create_fiber(fiber_params_t *params);
int switch_to_fiber(unsigned fid);
// implementation of fls
int fls_alloc(void);
int fls_free(long);
long fls_get(fls_params_t *);
int fls_set(fls_params_t *);
// not explicitly callable by user
int exit_fibered(void);
// kprobe handlers
int pre_exit_handler(struct kprobe *p, struct pt_regs *regs);
void post_exit_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags);

// utils
fibered_process_node_t *check_if_process_is_fibered(unsigned process_pid);
fiber_node_t *check_if_this_thread_is_fiber(fibered_process_node_t *fibered_process_node);
fiber_node_t *check_if_fiber_exist(fibered_process_node_t *fibered_process_node, unsigned fid);
unsigned long get_actual_fiber_time(fiber_node_t *current_fiber_node);

/**
 * @brief The state of the fiber
 *
 */
typedef enum fiber_state {
    IDLE,   /**< The fiber is created but no thread switched to it */
    RUNNING /**< The fiber is running since the thread switched to it */
} fiber_state_t;

/**
 * @brief Implementation of fiber local storage for fibers
 *
 * Each fiber has its own storage made of an array of longs, for knowing if a position of the array
 * is allocated or not, a bitmap is used.
 */
typedef struct fiber_local_storage {
    DECLARE_BITMAP(fls_bitmap, MAX_FLS); /**< The bitmap for the local storage, used for knowning
                                            which cells of the array are set*/
    long fls[MAX_FLS];                   /**< The actual array of the local storage */
} fiber_local_storage_t;

/**
 * @brief A node in the @ref fibers_list. This type fully represent a @c fiber
 *
 * This is the complete description of a fiber.
 */
typedef struct fiber {
    unsigned id;                        /**< Unique if of the fiber, used for ... */
    unsigned long entry_point;          /**< The starting fuction of the fiber */
    unsigned long base_user_stack_addr; /** The starting address of the stack that will be
                                              allocated by the library */
    fiber_state_t state;                /**< The current state of the fiber */
    struct pt_regs regs; /**< The current snapshot of cpu registers. The values that we user are:
- `regs->ip` - The current instruction pointer;
- `regs->sp` - The current stack pointer.
*/
    pid_t
        created_by; /**< The `pid` of the process (`tgid` of every thread) that created the fiber */
    pid_t run_by;   /**< This field represents:
    - the `pid` of the thread that is currently running the fiber if @fiber::state is @ref
    fiber_state::RUNNING
    - the `pid` of the last thread that executed it if @ref fiber_state::IDLE
    - -1 if we just called @ref create_fiber*/
    unsigned success_activations_count;  /**< Number of successful activation of the fiber */
    unsigned failed_activations_count;   /**< Number of failed activation of the fiber */
    unsigned long total_time;            /**< Total running time of the fiber */
    struct timespec time_last_switch;    /**< Time of when the last switch occurred*/
    struct list_head list;               /**< List implementation structure */
    fiber_local_storage_t local_storage; /**< Fiber local storage */

    struct fpu fpu_regs; /**< Used for saving the fpu registers*/
} fiber_node_t;

/**
 * @brief A generic list of fibers
 *
 * The list has the purpose of containing the list of fibers for a given process.
 *
 */
typedef struct fibers_list {
    struct list_head list;
    unsigned fibers_count; /**< Number of fibers created */
} fibers_list_t;

/**
 * @brief A node of the list of processes in @ref fibered_processes_list, a **fiber-enabled**
 * process
 *
 * A @ref fibered_process is a process in which at least one thread has been converted to a fiber,
 * so a fibered_process always has at least one fiber in the fibered_process::fibers_list field.
 * Moreover the first element of the list has fibered_process::prev pointing to @c NULL and the last
 * element of the list has fibered_process::next pointing to @c NULL.
 *
 */
typedef struct fibered_process {
    pid_t pid; /**< the pid of the main thread, so the tgid of every thread of the process */
    fibers_list_t fibers_list; /**< A list of fibers created in the process environment (so by any
                                  thread in it) */
#ifdef USE_HASH_LIST
    struct hlist_node hlist; /**< Hashlist implementation structure */
#else
    struct list_head list; /**< List implementation structure */
#endif
} fibered_process_node_t;

/**
 * @brief The list of processes that have at least one fiber
 *
 * This list has the purpose of containing the list of processes that have created at least one
 * fiber and they became fiber-enabled.
 *
 */
typedef struct fibered_processes_list {
#ifdef USE_HASH_LIST
    DECLARE_HASHTABLE(hash_table, HASH_KEY_SIZE);
#else
    struct list_head list;
#endif
    unsigned processes_count; /**< The number of elements in the list */
} fibered_processes_list_t;

#endif