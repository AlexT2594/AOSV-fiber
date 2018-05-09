/*
 * Originally from https://gist.github.com/brenns10/65d1ee6bb8419f96d2ae693eb7a66cc0
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.h"
#include "ioctlcmd.h"
#include <asm/current.h>
#include <asm/ptrace.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/hashtable.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/sched/task_stack.h>
#include <linux/semaphore.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define DEVICE_LOG ": DEV: "

int init_device(void);
void destroy_device(void);
// ioctl methods implementations
int convert_thread_to_fiber(void);

// structs
typedef struct fibered_process fibered_process_node_t;
typedef struct fiber_processes_list fiber_process_list_t;
typedef struct fiber fiber_node_t;
typedef struct fibers_list fibers_list_t;

#define FIBER_DEVICE_NAME "fiber"
#define BUF_LEN 80

typedef enum { RUNNING, IDLE } fiber_state_t;

/**
 * @brief The main structure of the fiber char device
 *
 */
typedef struct fiber_dev {
    struct miscdevice device;
} fiber_dev_t;

/**
 * @brief This is the node of the list of processes that have been registered as fiber-enabled. This also means that
 * their main thread has been converted to a fiber
 *
 */
typedef struct fibered_process {
    fibered_process_node_t *prev;
    fibered_process_node_t *next;
    fibers_list_t *fibers_list;
    pid_t pid; // tgid of the thread
} fibered_process_node_t;

typedef struct fibered_processes_list {
    fibered_process_node_t *head;
    fibered_process_node_t *tail;
    unsigned processes_count;
} fibered_processes_list_t;

/**
 * @brief This is the node of the list of the fibers that have been created by a process
 *
 */
typedef struct fiber {
    fiber_node_t *prev;
    fiber_node_t *next;
    unsigned id;
    void *starting_function;
    unsigned long base_user_stack_pointer;
    fiber_state_t state;
    struct pt_regs regs;
    pid_t created_by;
    unsigned success_activations_count;
    unsigned failed_activations_count;
    unsigned long total_time;
} fiber_node_t;

typedef struct fibers_list {
    fiber_node_t *head;
    fiber_node_t *tail;
    unsigned fibers_count;
} fibers_list_t;

#endif