#include "fiber.h"

int CreateFiber() {
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        exit(1);
    }
    int ret = ioctl(dev_fd, _IO(FIBER_IOC_MAGIC, FIBER_IOC_CREATEFIBER));
    if (ret < 0) {
        printf("Error while calling ioctl on fiber ERRNO %d\n", errno);
        exit(1);
    }
    return ret;
}