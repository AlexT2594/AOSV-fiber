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
    printf("Retvalue is %d", ret);
    return ret;
}