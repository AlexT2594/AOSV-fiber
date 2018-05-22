#include "fiber.h"

void safe_cleanup();

int ConvertThreadToFiber() {
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        exit(1);
    }
    int ret = ioctl(dev_fd, FIBER_IOC_CONVERTTHREADTOFIBER);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        exit(1);
    }
    printf("ConvertThreadToFiber retvalue is %d\n", ret);
    close(dev_fd);
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
    fiber_params_t *params = (fiber_params_t *)malloc(sizeof(fiber_params_t));
    params->function = (unsigned long)function;
    params->function_args = (unsigned long)args;
    params->stack_addr = (unsigned long)malloc(STACK_SIZE) + STACK_SIZE;
    // set the return address to the desired cleanup function, return address is the first cell of
    // the stack
    ((unsigned long *)params->stack_addr)[0] = (unsigned long)&safe_cleanup;

    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        exit(1);
    }
    int ret = ioctl(dev_fd, FIBER_IOC_CREATEFIBER, (unsigned long)params);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        exit(1);
    }
    printf("CreateFiber retvalue is %d\n", ret);
    close(dev_fd);
    return ret;
}

int SwitchToFiber(unsigned fid) {
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        exit(1);
    }
    int ret = ioctl(dev_fd, FIBER_IOC_SWITCHTOFIBER, (unsigned long)fid);
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        exit(1);
    }
    printf("SwitchToFiber retvalue is %d\n", ret);
    close(dev_fd);
    return ret;
}
// void *(*params)(void *)

/**
 * @brief Safely clean the memory when fiber accidentally ends
 *
 * The pointer of this function is set as return address of fiber function in CreateFiber
 *
 */
void safe_cleanup() {
    printf("Safe cleanup called\n");
    exit(0);
}