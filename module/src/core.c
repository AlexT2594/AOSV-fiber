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
static struct kprobe kp;

/**
 * @brief see Initialization of the module mutex
 * see https://www.kernel.org/doc/htmldocs/kernel-locking/Examples.html
 * It will be used everytime we have to access critical data structures
 */
static DEFINE_MUTEX(fiber_lock);

/**
 * @brief The variable of the core part that will contain the **fiber-enabled** processes
 */
// clang-format off
static fibered_processes_list_t fibered_processes_list = {
#ifdef USE_HASH_TABLE
    .hash_table = {[0 ...((1 << (HASH_KEY_SIZE)) - 1)] = HLIST_HEAD_INIT},
#else
    .list = LIST_HEAD_INIT(fibered_processes_list.list),
#endif
    .processes_count = 0
};
// clang-format on
EXPORT_SYMBOL(fibered_processes_list);

/*
 * Kprobe implementation
 */
int pre_exit_handler(struct kprobe *p, struct pt_regs *regs) {
    exit_fibered();
    // printk(KERN_DEBUG MODULE_NAME CORE_LOG "pre_exit_handler called by tgid %d",
    // current->tgid);

    return 0;
}
void post_exit_handler(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
    // printk(KERN_DEBUG MODULE_NAME CORE_LOG "pre_exit_handler called");
}

/**
 * @brief Init the core module
 *
 */
int init_core() {
    // Initalize the kprobe handler
    kp.pre_handler = pre_exit_handler;
    kp.post_handler = post_exit_handler;
    kp.addr = (kprobe_opcode_t *)kallsyms_lookup_name("do_exit");
    // de-comment the following line to register the kbrobe
    register_kprobe(&kp);
    return 0;
}

/**
 * @brief Destroy the core module
 *
 */
void destroy_core() { unregister_kprobe(&kp); }

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
 * - fiber::run_by is set to to `current->pid`;
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
    int ret;
    mutex_lock(&fiber_lock);
    // check if process if fiber enabled
    // check if process already created at least a fiber
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
// process has never created a fiber
#ifdef USE_HASH_LIST
        create_hash_entry(fibered_process_node, fibered_process_node_t,
                          fibered_processes_list.hash_table, &fibered_process_node->hlist,
                          current->tgid);
#else
        create_list_entry(fibered_process_node, &fibered_processes_list.list, list,
                          fibered_process_node_t);
#endif
        fibered_processes_list.processes_count++;
        fibered_process_node->pid = current->tgid;
        INIT_LIST_HEAD(&fibered_process_node->fibers_list.list);
        fibered_process_node->fibers_list.fibers_count = 0;
    }

    // check if the thread is already a fiber
    preempt_disable();
    fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (fiber_node == NULL) {
        // thread is not a fiber
        create_list_entry(fiber_node, &fibered_process_node->fibers_list.list, list, fiber_node_t);
        fiber_node->id = fibered_process_node->fibers_list.fibers_count;
        fiber_node->created_by = current->pid;
        fiber_node->run_by = current->pid;
        // -> FPU regs
        memcpy(&fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
        memset(&fiber_node->fpu_regs, 0, sizeof(struct fpu));
        fpu__initialize(&fiber_node->fpu_regs);
        // -> general purpose
        fiber_node->entry_point = fiber_node->regs.ip;
        fiber_node->success_activations_count = 1;
        fiber_node->failed_activations_count = 0;
        fiber_node->total_time = 0;
        getnstimeofday(&fiber_node->time_last_switch);
        fiber_node->state = RUNNING;
        fibered_process_node->fibers_list.fibers_count++;
        bitmap_clear(fiber_node->local_storage.fls_bitmap, 0, MAX_FLS);
        ret = fiber_node->id;
    } else
        ret = -ERR_THREAD_ALREADY_FIBER;

    preempt_enable();
    mutex_unlock(&fiber_lock);
    return ret;
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
 * - fiber::run_by is set to -1, indicating that no thread is running the fiber
 * - fiber::base_user_stack_addr is set to fiber_params::stack_addr - for setting the stack base
 * address
 * - fiber::regs::ip is set to fiber_params::function - for setting the starting instruction of the
 * fiber
 * - fiber::regs::di is set to fiber_params::function_args - for setting the first parameter of the
 * function that the user passed as starting point of the fiber
 * - fiber::created_by is set to `current->pid`;
 * - fiber::local_storage::fls_bitmap is cleared;
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
        printk(KERN_ALERT MODULE_NAME CORE_LOG "create_fiber() copy_from_user didn't copy %d bytes",
               ret);
        return -EFAULT;
    }

    mutex_lock(&fiber_lock);
    // check if process if fiber enabled
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // check if the thread is a fiber
    fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    // if (fiber_node == NULL) printk(KERN_ALERT "Thread %d is not a fiber", current->pid);
    if (fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }
    // add the node
    preempt_disable();

    create_list_entry(fiber_node, &fibered_process_node->fibers_list.list, list, fiber_node_t);
    fiber_node->id = fibered_process_node->fibers_list.fibers_count;
    fiber_node->created_by = current->pid;
    fiber_node->run_by = -1; // meaning no thread is running it
    fiber_node->state = IDLE;
    // -> Set the registers
    memcpy(&fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
    // -> FPU registers
    memset(&fiber_node->fpu_regs, 0, sizeof(struct fpu));
    fpu__initialize(&fiber_node->fpu_regs);
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
    bitmap_clear(fiber_node->local_storage.fls_bitmap, 0, MAX_FLS);
    ret = fiber_node->id;

    preempt_enable();
    mutex_unlock(&fiber_lock);
    return ret;
}

void print_fpu(struct fpu *fpu_regs) {
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "===== FPU DUMP START ====");
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "cwd :: %lu", (unsigned long)fpu_regs->state.fxsave.cwd);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "swd :: %lu", (unsigned long)fpu_regs->state.fxsave.swd);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "twd :: %lu", (unsigned long)fpu_regs->state.fxsave.twd);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "fcs :: %lu", (unsigned long)fpu_regs->state.fxsave.fcs);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "fip :: %lu", (unsigned long)fpu_regs->state.fxsave.fip);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "foo :: %lu", (unsigned long)fpu_regs->state.fxsave.foo);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "fop :: %lu", (unsigned long)fpu_regs->state.fxsave.fop);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "fos :: %lu", (unsigned long)fpu_regs->state.fxsave.fos);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "mxcsr :: %lu",
           (unsigned long)fpu_regs->state.fxsave.mxcsr);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "rdp :: %lu",
           (unsigned long)(unsigned long)fpu_regs->state.fxsave.rdp);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "rip :: %lu", (unsigned long)fpu_regs->state.fxsave.rip);
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "===== FPU DUMP END ====");
    return;
}

/**
 * @brief Switch to a chosen fiber
 *
 * @param fid
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
 * - FPU register are saved in the field @ref fiber::fpu_regs with the functions `fpu__save` and
 * `fpu__restore`
 *
 * Afterwards we'll get replace the current @c pt_regs structure with the one previously
 * saved.
 *
 * @return int 0 if everything went OK, otherwise:
 * - ERR_NOT_FIBERED if the the process is not fibered enabled, which means that none of its
 * threads has ever called @ref convert_thread_to_fiber
 * - ERR_NOT_FIBERED if the thread has never done @ref convert_thread_to_fiber
 * - ERR_FIBER_NOT_EXISTS if the fiber is not existing
 * - ERR_FIBER_ALREADY_RUNNING if the fiber is already running by another thread
 */
int switch_to_fiber(unsigned fid) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;
    fiber_node_t *requested_fiber_node;

    mutex_lock(&fiber_lock);
    // check if process is fiber enabled
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // check if the thread is a fiber
    current_fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (current_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // find a fiber element with id as fid
    requested_fiber_node = check_if_fiber_exist(fibered_process_node, fid);
    if (requested_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_FIBER_NOT_EXISTS;
    }
    if (requested_fiber_node->state == RUNNING) {
        mutex_unlock(&fiber_lock);
        requested_fiber_node->failed_activations_count += 1;
        return -ERR_FIBER_ALREADY_RUNNING;
    }

    // switch to that fiber
    preempt_disable();
    // update the total time, current fiber is always running
    current_fiber_node->total_time = get_actual_fiber_time(current_fiber_node);
    // update the switch time
    getnstimeofday(&current_fiber_node->time_last_switch);
#ifdef DEBUG
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "Total running time for fiber node %u is %lu",
           current_fiber_node->id, current_fiber_node->total_time);
#endif
    // update the fiber-to-come time, only if it was running
    if (requested_fiber_node->state == RUNNING)
        requested_fiber_node->total_time = get_actual_fiber_time(requested_fiber_node);
    // update the last switch for the fiber-to-come
    getnstimeofday(&requested_fiber_node->time_last_switch);

    current_fiber_node->state = IDLE;
    current_fiber_node->run_by = -1;
    requested_fiber_node->state = RUNNING;
    requested_fiber_node->run_by = current->pid;
    requested_fiber_node->success_activations_count += 1;
    // -> save the current registers
    memcpy(&current_fiber_node->regs, task_pt_regs(current), sizeof(struct pt_regs));
    // -> replace pt_regs
    memcpy(task_pt_regs(current), &requested_fiber_node->regs, sizeof(struct pt_regs));
    // -> FPU registers
    // dump the current fpu registers
    copy_fxregs_to_kernel(&current_fiber_node->fpu_regs);
    // replace the current with the requested fiber ones
    copy_kernel_to_fxregs(&requested_fiber_node->fpu_regs.state.fxsave);

#ifdef DEBUG
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "FID#%d->%d fpu to restore", current_fiber_node->id,
           requested_fiber_node->id);
    print_fpu(&requested_fiber_node->fpu_regs);

    struct fpu fpu_dump;
    memset(&fpu_dump, 0, sizeof(struct fpu));
    copy_fpregs_to_fpstate(&fpu_dump);

    printk(KERN_DEBUG MODULE_NAME CORE_LOG "FID#%d->%d actually restored", current_fiber_node->id,
           requested_fiber_node->id);
    print_fpu(&fpu_dump);
#endif

    preempt_enable();
    mutex_unlock(&fiber_lock);
    return 0;
}

/**
 * @brief Called when a process ends
 *
 * # Implementation
 * When a process ends we need to clear all the data structures that are associated with it, in
 * particular we need to remove all the @c fiber_node_t in the list of fibers of the process
 * @ref fibered_process_node_t::fibers_list and at the end the given @c fiber_node_t in the global
 * variable of type @ref fibers_list_t::list containing the list of all processes that are fiber
 * enabled. When a process ends, all of these operations need to be done automatically by the kernel
 * module. For this reason this function is used as a handler of a `kprobe` to the function
 * `do_exit`. Since this kind of handler cannot sleep, and we need to synchronize the access to
 * shared memory (essentially for checking if the process is a fiber), only the main threads of
 * every process that calls `do_exit` can enter here. Moreover race conditions that may affect
 * multiple fibered-processes are avoided since we loop on the fibered-processes list with the
 * utility function @ref list_for_each_entry_safe that is safe against removal while looping.
 *
 * @return int 0 if everything OK, otherwise @red ERR_NOT_FIBERED if the process is not a fiber
 */
int exit_fibered() {
    fibered_process_node_t *curr_process = NULL;
    fiber_node_t *curr_fiber = NULL;
    fiber_node_t *temp_fiber = NULL;

    // only the main thread can enter here
    if (current->tgid != current->pid) return 0;

    // get the process node
    curr_process = check_if_process_is_fibered(current->tgid);
    if (curr_process == NULL) return -ERR_NOT_FIBERED;

#ifdef DEBUG
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "Process pid %d request exit_fibered (tid#%d)",
           current->tgid, current->pid);
#endif

    if (!list_empty(&curr_process->fibers_list.list)) {
        list_for_each_entry_safe(curr_fiber, temp_fiber, &curr_process->fibers_list.list, list) {
            // remove fiber from list
            list_del(&curr_fiber->list);
            // free fiber
            kfree(curr_fiber);
        }
    }

#ifdef USE_HASH_LIST
    // remove process from hashlist
    hash_del(&curr_process->hlist);
#else
    // remove process from list
    list_del(&curr_process->list);
#endif
    // free process
    kfree(curr_process);

#ifdef DEBUG
    printk(KERN_DEBUG MODULE_NAME CORE_LOG "Process pid %d exited gracefully for ending thread %d",
           current->tgid, current->pid);
#endif

    return 0;
}

/**
 * @brief Allocate a new index if the local storage array
 *
 * # Implementation
 * The function simply check which is the first zero element in the fiber::local_storage::fls_bitmap
 * and if this element is within the maximum size of the local storage array, it sets it to 1 and
 * returns it.
 *
 * @return int The available index to be used for storage if everything OK otherwise:
 * - @ref ERR_NOT_FIBERED if the process is not fiber-enabled or the thread is not a fiber
 * - @ref ERR_FLS_FULL if there are no available position to be used for the fiber local storage
 */
int fls_alloc() {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;
    unsigned long index;

    mutex_lock(&fiber_lock);
    // check if process is fiber enabled
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }
    // check if the thread is a fiber
    current_fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (current_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // find the first zero element in the bitmap
    index =
        bitmap_find_next_zero_area(current_fiber_node->local_storage.fls_bitmap, MAX_FLS, 0, 1, 0);
    if (index > MAX_FLS) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_FULL;
    }
    // otherwise the index is available
    bitmap_set(current_fiber_node->local_storage.fls_bitmap, index, 1);
    mutex_unlock(&fiber_lock);
    return (long)index;
}

/**
 * @brief Allows for reusing the passed index by clearing it in the bitmap.
 *
 * # Implementation
 *
 * @param index The index of the fls entry to be freed
 * @return int 0 if everything OK, otherwise:
 * - @ref ERR_NOT_FIBERED if the process is not fiber-enabled or the thread is not a fiber
 * - @ref ERR_FLS_INVALID_INDEX if the index passed exceed the maximum size of the local storage or
 * it is associated to an entry that is not allocated
 */
int fls_free(long index) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;

    mutex_lock(&fiber_lock);
    // check if process is fiber enabled
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }
    // check if the thread is a fiber
    current_fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (current_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // check if index is valid
    if (index >= MAX_FLS) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }
    if (!test_bit(index, current_fiber_node->local_storage.fls_bitmap)) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }

    bitmap_clear(current_fiber_node->local_storage.fls_bitmap, index, 1);
    mutex_unlock(&fiber_lock);
    return 0;
}

/**
 * @brief Get the value of the fiber local storage at the given index
 *
 * # Implementation
 * The get of a local storage value is done by filling a data structure that is passed by the
 * userspace as @param params. For doing this we need to:
 * 1. Check the process and the thread are fibered
 * 2. Copy the passed data structure into kernel memory by using `copy_from_user`
 * 3. Check if the position is valid
 * 4. Get the value and update the local copy of the data structure
 * 5. Copy the local structure to the userspace with the function `copy_to_user`
 *
 * @param params @ref fls_params_t that need to have as @ref fls_params_t::idx the requested
 * position in the local storage. The field @ref fls_params_t::value will be filled by the kernel
 * with the corresponding value if everything is OK
 * @return long 0 if everything is OK, otherwise:
 * - @ref ERR_NOT_FIBERED if the process is not fiber-enabled or the thread is not a fiber
 * - @ref EFAULT if there was an error while reading the passed structure from userspace
 * - @ref ERR_FLS_INVALID_INDEX if the index passed exceed the maximum size of the local storage or
 * it is associated to an entry that is not allocated
 */
long fls_get(fls_params_t *params) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;
    fls_params_t k_params;
    int ret;
    // check if process is fiber enabled
    mutex_lock(&fiber_lock);
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }
    //  check if the thread is a fiber
    current_fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (current_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    ret = copy_from_user((void *)&k_params, (void *)params, sizeof(fls_params_t));
    if (ret != 0) {
        printk(KERN_ALERT MODULE_NAME CORE_LOG "fls_get() copy_from_user didn't copy %d bytes",
               ret);
        mutex_unlock(&fiber_lock);
        return -EFAULT;
    }
    // check if index is valid
    if (k_params.idx >= MAX_FLS) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }
    if (!test_bit(k_params.idx, current_fiber_node->local_storage.fls_bitmap)) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }

    k_params.value = current_fiber_node->local_storage.fls[k_params.idx];
    ret = copy_to_user((void *)params, (void *)&k_params, sizeof(fls_params_t));
    if (ret != 0) {
        printk(KERN_ALERT MODULE_NAME CORE_LOG "fls_get() copy_to_user didn't copy %d bytes", ret);
        mutex_unlock(&fiber_lock);
        return -EFAULT;
    }
    mutex_unlock(&fiber_lock);
    return 0;
}

/**
 * @brief Set the value of the local storage at given index
 *
 * # Implementation
 * When setting a value in the local storage we need to read a data structure passed from the user
 * space, so we need to:
 * 1. Copy the data structure in the kernel memory with `copy_from_user`
 * 2. Read the data and set the value in the local storage array
 *
 * @param params
 * @return int  long 0 if everything is OK, otherwise:
 * - @ref ERR_NOT_FIBERED if the process is not fiber-enabled or the thread is not a fiber
 * - @ref EFAULT if there was an error while reading the passed structure from userspace
 * - @ref ERR_FLS_INVALID_INDEX if the index passed exceed the maximum size of the local storage or
 * it is associated to an entry that is not allocated
 */
int fls_set(fls_params_t *params) {
    fibered_process_node_t *fibered_process_node;
    fiber_node_t *current_fiber_node;
    fls_params_t params_kern;
    int ret;
    mutex_lock(&fiber_lock);
    ret = copy_from_user(&params_kern, params, sizeof(fls_params_t));
    if (ret != 0) {
        printk(KERN_ALERT MODULE_NAME CORE_LOG "fls_set() copy_from_user didn't copy %d bytes",
               ret);
        mutex_unlock(&fiber_lock);
        return -EFAULT;
    }
    // check if process is fiber enabled
    fibered_process_node = check_if_process_is_fibered(current->tgid);
    if (fibered_process_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }
    // check if the thread is a fiber
    current_fiber_node = check_if_this_thread_is_fiber(fibered_process_node);
    if (current_fiber_node == NULL) {
        mutex_unlock(&fiber_lock);
        return -ERR_NOT_FIBERED;
    }

    // check if index is valid
    if (params_kern.idx >= MAX_FLS) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }
    // index must be one that has been previously given to the fiber
    if (!test_bit(params_kern.idx, current_fiber_node->local_storage.fls_bitmap)) {
        mutex_unlock(&fiber_lock);
        return -ERR_FLS_INVALID_INDEX;
    }
    current_fiber_node->local_storage.fls[params_kern.idx] = params_kern.value;
    mutex_unlock(&fiber_lock);
    return 0;
}

/*
 * Utils functions
 */

/**
 * @brief Check if a process is fibered
 *
 * # Implementation
 * For checking if the process is a fiber we need to find in the the @fibered_processes_list data
 * structure. This module implements the list of fibered process both as a linked structure than an
 * hash table. If `#define USE_HASH_TABLE` is present the hash table is used for checking if the
 * process is fibered, this means that the macro @ref check_if_exists_hash (that relies on kernel
 * built-in function `hash_for_each_possible_safe`) is used, otherwise we loop in a the linked list
 * of fibered processes by using the macro @ref check_if_exists (that relies on
 * `list_for_each_entry_safe`).
 *
 * @param process_pid The pid of the process to check
 * @return fibered_process_node_t* The pointer to the @ref fibered_process_node_t or NULL if process
 * is not fibered
 */
fibered_process_node_t *check_if_process_is_fibered(unsigned process_pid) {
    fibered_process_node_t *fibered_process_node;
#ifdef USE_HASH_LIST
    check_if_exists_hash(fibered_process_node, fibered_processes_list.hash_table, pid, process_pid,
                         hlist, fibered_process_node_t);
#else
    check_if_exists(fibered_process_node, &fibered_processes_list.list, pid, process_pid, list,
                    fibered_process_node_t);
#endif
    return fibered_process_node;
}

/**
 * @brief Check if the current thread is a fiber
 *
 * # Implementation
 * For checking if the current thread is a fiber we search for it in the linked list of fibers
 * belonging to the process. For doing this, the macro @ref check_if_exists is used.
 *
 * @param fiber_process_node The pointer to the element representing the current fibered process
 * @return fiber_node_t* A pointer to the fiber element in the list of fibers
 */
fiber_node_t *check_if_this_thread_is_fiber(fibered_process_node_t *fibered_process_node) {
    fiber_node_t *current_fiber_node;
    check_if_exists(current_fiber_node, &fibered_process_node->fibers_list.list, run_by,
                    current->pid, list, fiber_node_t);
    return current_fiber_node;
}

/**
 * @brief Check if the given @param fid is associated with an existing fiber
 *
 * # Implementation
 * For checking if a fiber exists we search for it in the linked list of fibers belonging to the
 * process passed as input. For doing this, the macro @ref check_if_exists is used.
 *
 * @param fibered_process_node The pointer to the element representing the current fibered process
 * @param fid The fiber id to check
 * @return fiber_node_t* A pointer to the fiber element in the list of fibers
 */
fiber_node_t *check_if_fiber_exist(fibered_process_node_t *fibered_process_node, unsigned fid) {
    fiber_node_t *current_fiber_node;
    check_if_exists(current_fiber_node, &fibered_process_node->fibers_list.list, id, fid, list,
                    fiber_node_t);
    return current_fiber_node;
}

/**
 * @brief Compute the actual live total time for fiber
 *
 * @param current_fiber_node
 * @return unsigned long
 */
unsigned long get_actual_fiber_time(fiber_node_t *current_fiber_node) {
    struct timespec current_time_s;
    unsigned long current_time, fiber_time;
    // if the fiber is idle no need to compute time
    if (current_fiber_node->state == IDLE) return current_fiber_node->total_time;
    // if the fiber is currently running, compute the time from the last switch to now
    getnstimeofday(&current_time_s);
    // compute times in millis
    current_time = current_time_s.tv_sec * 1000 + current_time_s.tv_nsec / 1000000;
    fiber_time = current_fiber_node->time_last_switch.tv_sec * 1000 +
                 current_fiber_node->time_last_switch.tv_nsec / 1000000;
    printk("INIT");
    printk("current_time = %lu", current_time);
    printk("fiber_time = %lu", fiber_time);
    printk("END");
    return current_fiber_node->total_time + (current_time - fiber_time);
}