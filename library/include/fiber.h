#ifndef __FIBER_H
#define __FIBER_H

#include "../../module/include/ioctlcmd.h"
#include <errno.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIBER_DEV_PATH "/dev/fiber"

#define STACK_SIZE 16384

int ConvertThreadToFiber();
int CreateFiber(void (*function)(void *), void *args);
int SwitchToFiber(unsigned fid);

int create_context();

#endif