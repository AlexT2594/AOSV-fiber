#include "core.h"

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
    fiber_t *fiber_node;
    // call ioctl
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        return -1;
    }
    int ret = ioctl(dev_fd, FIBER_IOC_CONVERTTHREADTOFIBER);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        return -1;
    }
    printf("ConvertThreadToFiber retvalue is %d\n", ret);
    close(dev_fd);
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
int CreateFiber(void *(*function)(void *), void *args) {
    fiber_t *fiber_node;
    // prepare the params
    fiber_params_t *params = (fiber_params_t *)malloc(sizeof(fiber_params_t));
    params->function = (unsigned long)function;
    params->function_args = (unsigned long)args;
    params->stack_addr = (unsigned long)malloc(STACK_SIZE) + STACK_SIZE;
    // -> set the return address to the desired cleanup function,
    //    return address is the first cell of the stack
    ((unsigned long *)params->stack_addr)[0] = (unsigned long)&safe_cleanup;

    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        return -1;
    }
    int ret = ioctl(dev_fd, FIBER_IOC_CREATEFIBER, (unsigned long)params);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        return -1;
    }
    printf("CreateFiber retvalue is %d\n", ret);
    close(dev_fd);
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
    fiber_t *fiber_node;
    // check if that fiber locally exists
    check_if_exists(fiber_node, &fibers_list.list, id, fid, list, fiber_t);
    if (fiber_node == NULL) {
        errno = ERR_FIBER_NOT_EXISTS;
        return -1;
    }
    // call ioctl
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        return -1;
    }
    int ret = ioctl(dev_fd, FIBER_IOC_SWITCHTOFIBER, (unsigned long)fid);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        return -1;
    }
    printf("SwitchToFiber retvalue is %d\n", ret);
    close(dev_fd);
    return ret;
}

/**
 * @brief Tells the kernel module that the process is going to terminate
 *
 * @return int
 */
int ExitFibered() {
    printf("Called ExitFibered\n");
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ", errno %d. Try again later.\n", errno);
        return -1;
    }
    int ret = ioctl(dev_fd, FIBER_IOC_EXIT);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        return -1;
    }
    close(dev_fd);
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
    printf("Safe cleanup called\n");
    ExitFibered();
    clean_memory();
    exit(0);
}

/**
 * @brief Clean all the memory structures created by the library
 *
 */
void clean_memory() {
    fiber_t *curr_fiber = NULL;
    fiber_t *temp_fiber = NULL;
    if (!list_empty(&fibers_list.list)) {
        list_for_each_entry_safe(curr_fiber, temp_fiber, &fibers_list.list, list) {
            // check if fiber is created with conver_thread_to_fiber
            if (curr_fiber->params != NULL) {
                // free stack
                if (curr_fiber->params->stack_addr != 0)
                    free((void *)(curr_fiber->params->stack_addr - STACK_SIZE));
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