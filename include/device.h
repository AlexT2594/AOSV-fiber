/*
 * Originally from https://gist.github.com/brenns10/65d1ee6bb8419f96d2ae693eb7a66cc0
 */

#ifndef __DEVICE_H
#define __DEVICE_H

#include "common.h"
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

int init_device(void);
void destroy_device(void);

#define SUCCESS 0
#define DEVICE_NAME "fiber"
#define BUF_LEN 80

#endif