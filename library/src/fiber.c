#include "fiber.h"

int CreateFiber() {
    int dev_fd = open(FIBER_DEV_PATH, O_RDWR, 0666);
    if (dev_fd < 0) {
        printf("Cannot open " FIBER_DEV_PATH ". Try again later.\n");
        exit(1);
    }
    int res = ioctl(dev_fd, _IO(FIBER_IOC_MAGIC, FIBER_IOC_CREATEFIBER));
    if (res < 0) {
        printf("Error while creating fiber");
        exit(1);
    }
    return res;
}