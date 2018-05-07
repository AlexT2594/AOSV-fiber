#ifndef __FIBER_H
#define __FIBER_H

#include "../../module/include/ioctlcmd.h"
#include <fcntl.h>
#include <linux/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <sys/stat.h>

#define FIBER_DEV_PATH "/dev/fiber"

int CreateFiber();

#endif