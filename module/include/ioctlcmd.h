// Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
// <alex.tufa94@gmail.com>
//
// This file is part of Fiber (Kernel Module).
//
// Fiber (Kernel Module) is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fiber (Kernel Module) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Fiber (Kernel Module).  If not, see <http://www.gnu.org/licenses/>.
//

/**
 * @brief This file contains definitions of ioctl commands
 *
 * This header file is imported also in the library.
 *
 * # Module operations
 * We define the following operations for the module that are mapped with the library (Library ->
 * Module)
 * - ConvertThreadToFiber -> FIBER_IOC_CONVERTTHREADTOFIBER
 * - CreateFiber -> FIBER_IOC_CREATEFIBER
 * - SwitchToFiber -> FIBER_IOC_SWITCHTOFIBER
 * - FlsAlloc -> FLS_ALLOC
 * - FlsFree -> FLS_FREE
 * - FlsGetValue -> FLS_GET
 * - FlsSetValue -> FLS_SET
 *
 * @file ioctlcmd.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-07
 */

#ifndef __IOCTLCMD_H
#define __IOCTLCMD_H

#include <linux/ioctl.h>

#define FIBER_IOC_MAGIC 0xF1

#define FIBER_IOCRESET _IO(FIBER_IOC_MAGIC, 0)
#define FIBER_IOC_CONVERTTHREADTOFIBER _IO(FIBER_IOC_MAGIC, 1)
#define FIBER_IOC_CREATEFIBER _IOW(FIBER_IOC_MAGIC, 2, int)
#define FIBER_IOC_SWITCHTOFIBER _IO(FIBER_IOC_MAGIC, 3)
#define FIBER_IOC_FLS_ALLOC _IOWR(FIBER_IOC_MAGIC, 4, int)
#define FIBER_IOC_FLS_FREE _IOW(FIBER_IOC_MAGIC, 5, int)
#define FIBER_IOC_FLS_GET _IOR(FIBER_IOC_MAGIC, 6, int)
#define FIBER_IOC_FLS_SET _IOW(FIBER_IOC_MAGIC, 7, int)
#define FIBER_IOC_EXIT _IO(FIBER_IOC_MAGIC, 8)
// the maximum number of syscall integer id
#define FIBER_IOC_MAXNR 8

// errors
#define ERR_THREAD_ALREADY_FIBER 100
#define ERR_NOT_FIBERED 200
#define ERR_FIBER_NOT_EXISTS 300
#define ERR_FIBER_ALREADY_RUNNING 400

/**
 * @brief Params to be passed by the library when creating or converting to a fiber
 *
 */
typedef struct fiber_params {
    unsigned long stack_addr; /**< The stack starting address allocated by the library */
    unsigned long function; /**< The function pointer passed by the user, that will be the starting
                               point of the fiber */
    unsigned long function_args; /**< Pointer to params for fiber_params::function */
} fiber_params_t;

#endif