/**
 * @brief This file contains definitions of ioctl commands
 *
 * @file ioctlcmd.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-07
 */
/*
 * IOCTL Section
 * We define the following operations for the module
 * - CONVERTTOFIBER - ConvertThreadToFiber
 * - CREATEFIBER - CreateFiber
 *
 * - SWITCHTOFIBER - SwitchToFiber
 * - FLS_ALLOC - FlsAlloc
 * - FLS_FREE - FlsFree
 * - FLS_GET - FlsGetValue
 * - FLS_SET - FlsSetValue
 */

#include <linux/ioctl.h>

#define FIBER_IOC_MAGIC 0xF1

#define FIBER_IOCRESET _IO(FIBER_IOC_MAGIC, 0)
#define FIBER_IOC_CONVERTTHREADTOFIBER _IO(FIBER_IOC_MAGIC, 1)
#define FIBER_IOC_CREATEFIBER _IO(FIBER_IOC_MAGIC, 2)
#define FIBER_IOC_SWITCHTOFIBER _IO(FIBER_IOC_MAGIC, 3)
#define FIBER_IOC_FLS_ALLOC _IOWR(FIBER_IOC_MAGIC, 4, int)
#define FIBER_IOC_FLS_FREE _IOW(FIBER_IOC_MAGIC, 5, int)
#define FIBER_IOC_FLS_GET _IOR(FIBER_IOC_MAGIC, 6, int)
#define FIBER_IOC_FLS_SET _IOW(FIBER_IOC_MAGIC, 7, int)

#define FIBER_IOC_MAXNR 7