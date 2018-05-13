/**
 * @brief This file contains definitions and macros for the *proc* part of the module
 *
 * @file proc.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#ifndef __PROC_H
#define __PROC_H

#include "common.h"
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_LOG ": PROC: "

/*
 * Exposed methods
 */

int init_proc(void);
void destroy_proc(void);

#define PROC_FOLDER "fibers"
#define PROC_ENTRY "fiber"

#endif