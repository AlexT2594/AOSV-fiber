// Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
// <alex.tufa94@gmail.com>
//
// This file is part of Fibers (Kernel Module).
//
// Fibers (Kernel Module) is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Fibers (Kernel Module) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Fibers (Kernel Module).  If not, see <http://www.gnu.org/licenses/>.
//

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
#include "core.h"
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