#ifndef __PROC_H
#define __PROC_H

#include "common.h"
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define PROC_LOG ": PROC: "

int init_proc(void);
void destroy_proc(void);

#define PROC_FOLDER "fibers"
#define PROC_ENTRY "fiber"

#endif