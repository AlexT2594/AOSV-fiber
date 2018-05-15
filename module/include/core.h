
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
#include <asm/current.h>
#include <asm/ptrace.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/sched/task_stack.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define CORE_LOG ": DEV: "

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

int convert_thread_to_fiber(void);
int create_fiber(fiber_params_t *params);
int switch_to_fiber(int fiber_id);

/**
 * @brief Params to be passed by the library when creating or converting to a fiber
 *
 */
typedef struct fiber_params {
    unsigned long stack_addr; /**< The stack starting address allocated by the library */
    unsigned long function; /**< The function pointer passed by the user, that will be the starting
                               point of the fiber */
    unsigned long function_args; /**< Pointer to params for fiber_params::function */
} fiber_params_t;

/**
 * @brief The state of the fiber
 *
 */
typedef enum fiber_state {
    RUNNING, /**< The fiber is running since the thread switched to it */
    IDLE     /**< The fiber is created but no thread switched to it */
} fiber_state_t;

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
    fibered_process_node_t *prev; /**< The previous element of the list */
    fibered_process_node_t *next; /**< The next element of the list */
    fibers_list_t *fibers_list; /**< A list of fibers created in the process environment (so by any
                                   thread in it) */
    pid_t pid; /**< the pid of the main thread, so the tgid of every thread of the process */
} fibered_process_node_t;

/**
 * @brief The list of processes that have at least one fiber
 *
 * This list has the purpose of containing the list of processes that have created at least one
 * fiber and they became fiber-enabled. The list can be considered empty when at least one of
 * fibered_processes_list::head or fibered_processes_list::tail is pointing to @c NULL.
 *
 */
typedef struct fibered_processes_list {
    fibered_process_node_t *head; /**< The first element in the list */
    fibered_process_node_t *tail; /**< The last element in the list */
    unsigned processes_count;     /**< The number of elements in the list */
} fibered_processes_list_t;

/**
 * @brief A node in the @ref fibers_list. This type fully represent a @c fiber
 *
 * This is the complete description of a fiber.
 *
 */
typedef struct fiber {
    fiber_node_t *prev;                 /**< Pointer to the previous element in the list */
    fiber_node_t *next;                 /**< Pointer to the next element in the list */
    unsigned id;                        /**< Unique if of the fiber, used for  */
    unsigned long starting_function;    /**< The starting fuction of the fiber */
    unsigned long base_user_stack_addr; /** The starting address of the stack that will be
                                              allocated by the library */
    fiber_state_t state;                /**< The current state of the fiber */
    struct pt_regs regs; /**< The current snapshot of cpu registers. The values that we user are:
- `regs->ip` - The current instruction pointer;
- `regs->sp` - The current stack pointer.
*/
    pid_t
        created_by; /**< The `pid` of the process (`tgid` of every thread) that created the fiber */
    unsigned success_activations_count; /** Number of successful activation of the fiber */
    unsigned failed_activations_count;  /** Number of failed activation of the fiber */
    unsigned long total_time;           /** Total running time of the fiber */
} fiber_node_t;

/**
 * @brief A generic list of fibers
 *
 * The list has the purpose of containing the list of fibers for a given process.  The list can be
 * considered empty when at least one of fibers_list::head or fibers_list::tail is pointing to @c
 * NULL.
 *
 */
typedef struct fibers_list {
    fiber_node_t *head;    /**< The first element in the list */
    fiber_node_t *tail;    /**< The last element in the list */
    unsigned fibers_count; /**< Number of fibers created */

} fibers_list_t;

#endif