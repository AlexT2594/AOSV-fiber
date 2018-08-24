// Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
// <alex.tufa94@gmail.com>
//
// This file is part of Fibers (Library).
//
// Fibers (Library) is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fibers (Library) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Fibers (Library).  If not, see <http://www.gnu.org/licenses/>.
//

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

#define CORE_TAG "CORE: "

#include "../../module/include/ioctlcmd.h"
#include "common.h"
#include "fiber.h"
#include "list.h"
#include "utils.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stropts.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
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
int open_device();

void start(void) __attribute__((constructor));
void end(void) __attribute__((destructor));

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