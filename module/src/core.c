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
 * When a thread is converted to a fiber several tasks are performed. First of all, we must check if
 * the process if *fiber-enabled* or not by searching if its `pid` (so even its `tgid`) is already
 * present in the fibered_processes_list, that must be initialized if it is the case.
 *
 * ## Process not **fiber-enabled**
 * If the process never created a fiber, it must become **fiber-enabled**, this means that we have
 * to instantiate a fibered_process element in the @ref fibered_processes_list variable. Then we
 * have to instantiate a @ref fiber element in the fibers_list field of the fibered_process element
 * of the list.
 *
 * ## Process **fiber-enabled**
 * If the process is already a *fiber-enabled* then we have just to append a new @ref fiber entry to
 * the fibered_process::fibers_list. This requires that the parameters are also set according to the
 * ones that are passed to the function as @p params argument. At the end a new proc file must be
 * created in the directory `/proc/<pid>/fibers/<fid>`. The fields of the fiber element are set in
 * this way:
 * - fiber::id is set to the current number of fibers -1;
 * - fiber::regs is set with the function `task_pt_regs(current)`;
 * - fiber::starting_function is set to fiber::regs::ip;
 * - fiber::state is set to fiber_state::RUNNING;
 * - fiber::created_by is set to `current->pid`;
 * - fiber::success_activations_count is set to 1 if success else 0;
 * - fiber::failed_activations_count is set to 1 if fail else 0;
 * - fiber::total_time is set to 0;
 * - fiber::base_user_stack_addr is ignored;
 *
 * @return int the id of the newly created fiber otherwise `ERR_THREAD_ALREADY_FIBER` if the thread
 * already has been converted to a fiber
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

    // TODO Create proc now

    return fiber_node->id;
}

/**
 * @brief Create a new fiber
 *
 * # Implementation
 * First of all, only a thread that has been converted to a fiber can create fibers, so after this
 * check we have to create a @ref fiber element in the fibered_process::fibers_list and assign to it
 * the params that are passed in the @p params argument, so:
 * - fiber::id is set to the current number of fibers -1;
 * - fiber::regs is set with the function `task_pt_regs(current)`;
 * - fiber::starting_function is set to fiber::regs::ip;
 * - fiber::state is set to fiber_state::IDLE;
 * - fiber::base_user_stack_addr is set to fiber_params::stack_addr - for setting the stack base
 * address
 * - fiber::regs::ip is set to fiber_params::function - for setting the starting instruction of the
 * fiber
 * - fiber::regs::di is set to fiber_params::function_args - for setting the first parameter of the
 * function that the user passed as starting point of the fiber
 * - fiber::created_by is set to `current->pid`;
 * - fiber::success_activations_count is set 0;
 * - fiber::failed_activations_count is set 0;
 * - fiber::total_time is set to 0;
 *
 * At the end a proc directory is created in `/proc/<pid>/fibers/<fid>`.
 *
 * @param params
 * @return int the id of the newly created fiber otherwise ERR_NOT_FIBERED if the thread is not
 * *fiber-enabled*
 */
int create_fiber(fiber_params_t *params) { return 0; }

/**
 * @brief
 *
 * @param fiber_id
 * @return int
 */
int switch_to_fiber(int fiber_id) { return 0; }