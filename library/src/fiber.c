#include "fiber.h"

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

int CreateFiber(void (*function)(void *), void *args) {
    fiber_params_t *params = (fiber_params_t *)malloc(sizeof(fiber_params_t));
    params->function = (unsigned long)function;
    params->function_args = (unsigned long)args;
    params->stack_addr = (unsigned long)malloc(STACK_SIZE);
    printf("Im passing function address %lu\n", (unsigned long)function);

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