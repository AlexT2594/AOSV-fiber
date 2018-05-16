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

#define FIBER_IOC_MAXNR 7

// errors
#define ERR_THREAD_ALREADY_FIBER 100
#define ERR_NOT_FIBERED 200
#define ERR_FIBER_NOT_EXISTS 300
#define ERR_FIBER_ALREADY_RUNNING 400