/**
 * @brief This file contains the implementation of all the core functions of the module
 *
 * @file core.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#include "core.h"

/*
 * Variables
 */

/**
 * @brief The variable of the core part that will contain the **fiber-enabled** processes
 *
 */
// clang-format off
static fibered_processes_list_t fibered_processes_list = {
    .head = NULL,
    .tail = NULL,
    .processes_count = 0
};
// clang-format on

/*
 * Implementations
 */

/**
 * @brief Convert the current thread to a fiber
 *
 * # Implementation
 * When a thread is converted to a fiber several task are performed. First of all, if the process
 * never created a fiber, it must become **fiber-enabled**, this means that we have to instantiate a
 * fibered_process element in the @ref fibered_processes_list variable. Then we have to
 * instantiate a @ref fiber element in the fibers_list field of the fibered_process element of the
 * list.
 *
 * @return int 0 if everything ok, otherwise an error listed in @ref ioctlcmd.h
 */
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
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "tgid is %d", fibered_process_node->pid);
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
        if (fibered_process_node->fibers_list->head == NULL)
            fibered_process_node->fibers_list->head = fiber_node;
        if (fibered_process_node->fibers_list->tail != NULL) {
            fibered_process_node->fibers_list->tail->next = fiber_node;
            fiber_node->prev = fibered_process_node->fibers_list->tail;
        }
        fibered_process_node->fibers_list->tail = fiber_node;
    } else
        return -ERR_THREAD_ALREADY_FIBER;

    fiber_node->id = fibered_process_node->fibers_list->fibers_count - 1;
    fiber_node->state = RUNNING;

    return 0;
}
