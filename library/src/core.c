/**
 * Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
 * <alex.tufa94@gmail.com>
 *
 * This file is part of Fibers (Library).
 *
 * Fibers (Library) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fibers (Library) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fibers (Library).  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/**
 * @brief This file represents the core of the library
 *
 * @file core.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-07-18
 */

#include "core.h"
#include <semaphore.h>

sem_t device_sem;

int fiber_dev_fd = -1;

/*
 * Private member variables
 */
// clang-format off
static fibers_list_t fibers_list = {
    .list = LIST_HEAD_INIT(fibers_list.list),
    .fibers_count = 0
};
// clang-format on

/*
 * Syscalls implementation
 */

int ConvertThreadToFiber() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "ConvertThreadToFiber()\n");
#endif
    fiber_t *fiber_node;
    // only one thread at a time can open the device
    sem_wait(&device_sem);
    int dev_fd = open_device();
    sem_post(&device_sem);
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_CONVERTTHREADTOFIBER);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "ConvertThreadToFiber() ioctl error, errno %d\n", errno);
        return -1;
    }
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "ConvertThreadToFiber() assigned %d to fiber\n", ret);
#endif
    // add new node to the list of fiber
    create_list_entry(fiber_node, &fibers_list.list, list, fiber_t);
    fiber_node->id = ret;
    fiber_node->params = NULL;
    return ret;
}

/**
 * @brief Create a Fiber object
 *
 * # Implementation
 * When we create a new fiber you have to pass the following parameters to the kernel:
 * - fiber_params::function - the address of the starting function of the fiber
 * - fiber_params::function_args - the address of arguments allocated by the user and that will be
 * passed to the function of the fiber
 * - fiber_params::stack_addr - the stack address allocated by the library (**Remember**: since the
 * malloc function returns the starting address of the allocated memory and the stack grows in
 * reverse, we have to pass the ending pointer of the memory allocated by the library)
 *
 * @param function
 * @param args
 * @return int
 */
int CreateFiber(unsigned long stack_size, void *(*function)(void *), void *args) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "CreateFiber()\n");
#endif
    fiber_t *fiber_node;
    // prepare the params
    fiber_params_t *params = (fiber_params_t *)malloc(sizeof(fiber_params_t));
    params->function = (unsigned long)function;
    params->function_args = (unsigned long)args;
    // params->stack_addr = (unsigned long)malloc(stack_size) + stack_size;
    params->stack_addr = (unsigned long)aligned_alloc(16, stack_size) + stack_size;
#ifdef DEBUG
    printf("Is stack aligned: %d\n", params->stack_addr % 16 == 0);
    printf("Stack addr is %lu\n", params->stack_addr);
#endif
    params->stack_size = stack_size;
    // -> set the return address to the desired cleanup function,
    //    return address is the first cell of the stack, this an implementation what the call
    //    instruction should have done. The following two lines are equivalent to the push of the
    //    ret address
    ((unsigned long *)params->stack_addr)[0] = (unsigned long)&safe_cleanup;
    params->stack_addr -= 8;

    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_CREATEFIBER, (unsigned long)params);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "CreateFiber() ioctl error, errno %d\n", errno);
        return -1;
    }
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "CreateFiber() assigned %d to fiber\n", ret);
#endif
    // add new node to the list of fiber
    create_list_entry(fiber_node, &fibers_list.list, list, fiber_t);
    fiber_node->id = ret;
    fiber_node->params = params;
    return ret;
}

/**
 * @brief Switch to the passed fiber
 *
 * @param fid
 * @return int
 */
int SwitchToFiber(unsigned fid) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "SwitchToFiber(%u)\n", fid);
#endif
    fiber_t *fiber_node;
    // check if that fiber locally exists
    check_if_exists(fiber_node, &fibers_list.list, id, fid, list, fiber_t);
    if (fiber_node == NULL) {
        errno = ERR_FIBER_NOT_EXISTS;
        return -1;
    }
    // call ioctl
    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_SWITCHTOFIBER, (unsigned long)fid);
    if (ret < 0) {
        // printf(LIBRARY_TAG CORE_TAG "SwitchToFiber() ioctl error, errno %d\n", errno);
        return -1;
    }
    return ret;
}

/**
 * @brief Tells the kernel module that the process is going to terminate
 *
 * @return int
 */
int ExitFibered() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "ExitFibered()\n");
#endif
    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_EXIT);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "ExitFibered() ioctl error, errno %d\n", errno);
        return -1;
    }
    return ret;
}

/**
 * @brief Allocate a new storage cell in the local storage array
 *
 * @return long
 */
long FlsAlloc() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "FlsAlloc()\n");
#endif
    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_FLS_ALLOC);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "FlsAlloc() ioctl error, errno %d\n", errno);
        return -1;
    }
    return ret;
}

/**
 * @brief Free the given index in the local storage
 *
 * @param index
 * @return int
 */
int FlsFree(long index) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "FlsFree(%ld)\n", index);
#endif
    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_FLS_FREE, (unsigned long)index);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "FlsFree() ioctl error, errno %d\n", errno);
        return -1;
    }
    return ret;
}

/**
 * @brief Get a value from the local storage
 *
 * @param index
 * @return long
 */
long FlsGetValue(long index) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "FlsGetValue(%ld)\n", index);
#endif
    fls_params_t *req_params = (fls_params_t *)malloc(sizeof(fls_params_t));
    req_params->idx = index;
    req_params->value = 0;

    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_FLS_GET, (unsigned long)req_params);
    if (ret < 0) {
        printf("Is fd#%d valid::%d\n", dev_fd, fcntl(dev_fd, F_GETFD));
        printf(LIBRARY_TAG CORE_TAG "FlsGetValue(%ld) ioctl to %d error, errno %d\n", index, dev_fd,
               errno);
        return -1;
    }
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "FlsGetValue(%ld) = %lu\n", index, req_params->value);
#endif
    free(req_params);
    return req_params->value;
}

/**
 * @brief Set a value in the local storage
 *
 * @param index
 * @param value
 * @return int
 */
int FlsSetValue(long index, long value) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "FlsSetValue(%ld,%ld)\n", index, (unsigned long)value);
#endif
    // prepare the params
    fls_params_t *params = (fls_params_t *)malloc(sizeof(fls_params_t));
    params->idx = index;
    params->value = value;

    int dev_fd = open_device();
    if (dev_fd < 0 || fcntl(dev_fd, F_GETFD) < 0) return -1;
    int ret = ioctl(dev_fd, FIBER_IOC_FLS_SET, (unsigned long)params);
    if (ret < 0) {
        printf(LIBRARY_TAG CORE_TAG "FlsSetValue(%ld,%ld) ioctl error, errno %d\n", index, value,
               errno);
        return -1;
    }
    free(params);
    return ret;
}

/*
 * Utils functions
 */

/**
 * @brief Safely clean the memory when fiber accidentally ends
 *
 * The pointer of this function is set as return address of fiber function in CreateFiber
 *
 */
void safe_cleanup() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "safe_cleanup()\n");
#endif
    ExitFibered();
    clean_memory();
    exit(0);
}

/**
 * @brief Clean all the memory structures created by the library
 *
 */
void clean_memory() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "clean_memory()\n");
#endif
    fiber_t *curr_fiber = NULL;
    fiber_t *temp_fiber = NULL;
    if (!list_empty(&fibers_list.list)) {
        list_for_each_entry_safe(curr_fiber, temp_fiber, &fibers_list.list, list) {
            // check if fiber is created with conver_thread_to_fiber
            if (curr_fiber->params != NULL) {
                // free stack
                if (curr_fiber->params->stack_addr != 0) {
                    // emulates the pop %rbp
                    curr_fiber->params->stack_addr += 8;
                    free((void *)(curr_fiber->params->stack_addr - curr_fiber->params->stack_size));
                }
                // free params
                free(curr_fiber->params);
            }
            // remove fiber from list
            list_del(&curr_fiber->list);
            // free fiber
            free(curr_fiber);
        }
    }
}

/**
 * @brief Open the fiber device
 *
 * @return int
 */
int open_device() {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "open_device()\n");
#endif
    if (fiber_dev_fd < 0) {
        fiber_dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
        if (fiber_dev_fd < 0) {
            printf(LIBRARY_TAG CORE_TAG "Cannot open " FIBER_DEV_PATH ", errno %d.\n", errno);
            return -1;
        }
    }
    return fiber_dev_fd;
}

/**
 * @brief
 *
 */

__attribute__((constructor)) void start(void) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "start() constructor called\n");
#endif
    sem_init(&device_sem, 0, 1);
}

/**
 * @brief
 *
 */
__attribute__((destructor)) void end(void) {
#ifdef DEBUG
    printf(LIBRARY_TAG CORE_TAG "end() destructor called\n");
#endif
    close(fiber_dev_fd);
    clean_memory();
}