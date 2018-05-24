/**
 * @brief Header of core functions of the library
 *
 * @file core.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-24
 */
#ifndef __CORE_H
#define __CORE_H

#include "../../module/include/ioctlcmd.h"
#include "fiber.h"
#include "list.h"
#include "utils.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#define FIBER_DEV_PATH "/dev/fiber"

/**
 * @brief The number of unsigned long cells that the stack will contain
 *
 */
#define STACK_SIZE 2048 * 8

void safe_cleanup();
void clean_memory();

/**
 * @brief Single node in a fiber_list_t list
 *
 */
typedef struct fiber {
    unsigned id;
    fiber_params_t *params;
    struct list_head list;
} fiber_t;

/**
 * @brief A local list of fibers
 *
 */
typedef struct fibers_list {
    struct list_head list;
    unsigned fibers_count;
} fibers_list_t;

#endif