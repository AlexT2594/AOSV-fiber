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

// clang-format off
static fibered_processes_list_t fibered_processes_list = {
    .head = NULL,
    .tail = NULL,
    .processes_count = 0
};
// clang-format on
*/

/**
 * @brief see Initialization of the module mutex
 * see https://www.kernel.org/doc/htmldocs/kernel-locking/Examples.html
 * It will be used everytime we have to access critical data structures
 */
static DEFINE_MUTEX(fiber_lock);

static fibered_processes_list_t fibered_processes_list = {
    .list = LIST_HEAD_INIT(fibered_processes_list.list),
    .processes_count = 0,
};

/*
 * Implementations
 */

/**
 * @brief Convert the current thread to a fiber
 *
 * # Implementation
 * When a thread is converted to a fiber several tasks are performed. First of all, we must
 * check if the process if *fiber-enabled* or not by searching if its `pid` (so even its `tgid`)
 * is already present in the fibered_processes_list, that must be initialized if it is the case.
 *
 * ## Process not **fiber-enabled**
 * If the process never created a fiber, it must become **fiber-enabled**, this means that we
 * have to instantiate a fibered_process element in the @ref fibered_processes_list variable.
 * Then we have to instantiate a @ref fiber element in the fibers_list field of the
 * fibered_process element of the list.
 *
 * ## Process **fiber-enabled**
 * If the process is already a *fiber-enabled* then we have just to append a new @ref fiber
 * entry to the fibered_process::fibers_list. This requires that the parameters are also set
 * according to the ones that are passed to the function as @p params argument. At the end a new
 * proc file must be created in the directory `/proc/<pid>/fibers/<fid>`. The fields of the
 * fiber element are set in this way:
 * - fiber::id is set to the current number of fibers -1;
 * - fiber::regs is set with the function `task_pt_regs(current)`;
 * - fiber::entry_point is set to fiber::regs::ip;
 * - fiber::state is set to fiber_state::RUNNING;
 * - fiber::created_by is set to `current->pid`;
 * - fiber::success_activations_count is set to 1 if success else 0;
 * - fiber::failed_activations_count is set to 1 if fail else 0;
 * - fiber::total_time is set to 0;
 * - fiber::base_user_stack_addr is ignored - because the stack address is saved in `regs`;
 *
 * @return int the id of the newly created fiber otherwise `ERR_THREAD_ALREADY_FIBER` if the
 * thread already has been converted to a fiber
 */
int convert_thread_to_fiber() {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *fiber_node;
    // check if process if fiber enabled
    // check if process already created at least a fiber
    check_if_exists(fibered_process_node, &fibered_processes_list.list, pid, current->tgid, list,
                    fibered_process_node_t);
    if (fibered_process_node == NULL) {
        // process has never created a fiber
        printk(KERN_DEBUG MODULE_NAME CORE_LOG "Fibered process does not exist");
        create_list_entry(fibered_process_node, &fibered_processes_list.list, list,
                          fibered_process_node_t);
        fibered_processes_list.processes_count++;
        fibered_process_node->pid = current->tgid;
        INIT_LIST_HEAD(&fibered_process_node->fibers_list.list);
        fibered_process_node->fibers_list.fibers_count = 0;
    }

    // check if the thread is already a fiber
    check_if_exists(fiber_node, &fibered_process_node->fibers_list.list, run_by, current->pid, list,
                    fiber_node_t);
    if (fiber_node == NULL) {
        // thread is not a fiber
        create_list_entry(fiber_node, &fibered_process_node->fibers_list.list, list, fiber_node_t);
        fiber_node->id = fibered_process_node->fibers_list.fibers_count;
        fiber_node->created_by = current->pid;
        fiber_node->run_by = current->pid;
        memcpy(&fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
        fiber_node->entry_point = fiber_node->regs.ip;
        fiber_node->success_activations_count = 1;
        fiber_node->failed_activations_count = 0;
        fiber_node->total_time = 0;
        fiber_node->state = RUNNING;
        fibered_process_node->fibers_list.fibers_count++;
        return fiber_node->id;
    } else
        return -ERR_THREAD_ALREADY_FIBER;
}

/**
 * @brief Create a new fiber
 *
 * # Implementation
 * First of all, only a thread that has been converted to a fiber can create fibers, so after this
 * check we have to create a @ref fiber element in the fibered_process::fibers_list and assign to it
 * the params that are passed in the @p params argument, so:
 * - fiber::id is set to the current number of fibers -1;
 * - fiber::regs is set with the values of the @c pt_regs structure, after they have been obtained
 * with the function `task_pt_regs(current)`. In order to do this we have to allocate some kernel
 * memory using @c kmalloc;
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
int create_fiber(fiber_params_t *params) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *fiber_node;
    fiber_params_t params_kern;
    int ret;
    ret = copy_from_user(&params_kern, params, sizeof(fiber_params_t));
    if (ret != 0) {
        printk(KERN_ALERT MODULE_NAME CORE_LOG "copy_from_user didn't copy %d bytes", ret);
        return -EFAULT;
    }
    // check if process if fiber enabled
    check_if_exists(fibered_process_node, &fibered_processes_list.list, pid, current->tgid, list,
                    fibered_process_node_t);
    if (fibered_process_node == NULL) return -ERR_NOT_FIBERED;
    // check if the thread is a fiber
    check_if_exists(fiber_node, &fibered_process_node->fibers_list.list, run_by, current->pid, list,
                    fiber_node_t);
    if (fiber_node == NULL) return -ERR_NOT_FIBERED;
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "create_fiber Passed params_kern->function is %lu",
           params_kern.function);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "create_fiber Passed params_kern->stack_addr is %lu",
           params_kern.stack_addr);
    // create the fiber node
    create_list_entry(fiber_node, &fibered_process_node->fibers_list.list, list, fiber_node_t);
    fiber_node->id = fibered_process_node->fibers_list.fibers_count;
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "create_fiber Created fiber id is %u", fiber_node->id);
    fiber_node->created_by = current->pid;
    fiber_node->state = IDLE;
    // -> Set the registers
    memcpy(&fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
    fiber_node->regs.ip = params_kern.function;
    fiber_node->regs.di = params_kern.function_args;
    fiber_node->regs.sp = params_kern.stack_addr;
    fiber_node->regs.bp = params_kern.stack_addr;
    fiber_node->base_user_stack_addr = params_kern.stack_addr;
    // -> Save as reference
    fiber_node->entry_point = params_kern.function;
    fibered_process_node->fibers_list.fibers_count++;
    fiber_node->success_activations_count = 0;
    fiber_node->failed_activations_count = 0;
    fiber_node->total_time = 0;
    return fiber_node->id;
}

/**
 * @brief
 *
 * @param Switch to a chosen fiber
 *
 * #Implementation
 * Before starting the actual function, we have to do some checks:
 * 1. Check if there exists a process in the @ref fibered_processes_list, which means there
 * exists a process with pid equal to the tgid of the thread (this means that the processes
 * is _fibered-enabled_)
 * 2. Check if the @c pid of the currently running thread is present in at least one of @ref
 * fibered_process::fibers_list::fiber::created_by, this means that this thread has called
 * @ref
 * @convert_thread_to_fiber
 * 3. Check if there exists a @ref fiber element with a fiber::id equal to @p fid
 * 4. Check if the fiber is already @ref fiber_state::RUNNING
 *
 * The context of the currently running fiber has to be saved. This means:
 * - the @c pt_regs structure has to be saved to the current @ref fiber::regs
 * - TODO FPU @see https://wiki.osdev.org/SSE
 *
 * Afterwards we'll get replace the current @c pt_regs structure with the one previously
 * saved.
 *
 * @return int 0 if everything went OK, otherwise:
 * - ERR_NOT_FIBERED if the the process is not fibered enabled, which means that none of its
 * threads has ever called @ref convert_thread_to_fiber
 * - ERR_NOT_FIBERED if the thread has never done @ref convert_thread_to_fiber
 * - ERR_FIBER_NOT_EXISTS
 * - ERR_FIBER_ALREADY_RUNNING
 */
int switch_to_fiber(unsigned fid) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;
    fiber_node_t *requested_fiber_node;
    // check if process if fiber enabled
    check_if_exists(fibered_process_node, &fibered_processes_list.list, pid, current->tgid, list,
                    fibered_process_node_t);
    if (fibered_process_node == NULL) return -ERR_NOT_FIBERED;
    // check if the thread is a fiber
    check_if_exists(current_fiber_node, &fibered_process_node->fibers_list.list, run_by,
                    current->pid, list, fiber_node_t);
    if (current_fiber_node == NULL) return -ERR_NOT_FIBERED;
    // find a fiber element with id as fid
    check_if_exists(requested_fiber_node, &fibered_process_node->fibers_list.list, id, fid, list,
                    fiber_node_t);
    if (requested_fiber_node == NULL) return -ERR_FIBER_NOT_EXISTS;
    if (requested_fiber_node->state == RUNNING) return -ERR_FIBER_ALREADY_RUNNING;
    // switch to that fiber
    current_fiber_node->state = IDLE;
    requested_fiber_node->state = RUNNING;
    // -> save the current registers
    // TODO: FPU registers
    memcpy(&current_fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
    // -> replace pt_regs
    memcpy(task_pt_regs(current), &requested_fiber_node->regs, sizeof(struct pt_regs));
    // close the device descriptor
    close_device_descriptor();
    return 0;
}

/**
 * @brief Called when a process ends
 *
 * @return int
 */
int exit_fibered() {
    fibered_process_node_t *curr_process = NULL;
    fiber_node_t *curr_fiber = NULL;
    fiber_node_t *temp_fiber = NULL;
    // get the process node
    check_if_exists(curr_process, &fibered_processes_list.list, pid, current->tgid, list,
                    fibered_process_node_t);
    if (curr_process == NULL) { return ERR_NOT_FIBERED; }

    mutex_lock(&fiber_lock);
    if (!list_empty(&curr_process->fibers_list.list)) {
        list_for_each_entry_safe(curr_fiber, temp_fiber, &curr_process->fibers_list.list, list) {
            // remove fiber from list
            list_del(&curr_fiber->list);
            // free fiber
            kfree(curr_fiber);
        }
    }
    // remove process from list
    list_del(&curr_process->list);
    // free process
    kfree(curr_process);
    mutex_unlock(&fiber_lock);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "Process pid %d exited gracefully for ending thread %d",
           current->tgid, current->pid);
    return 0;
}