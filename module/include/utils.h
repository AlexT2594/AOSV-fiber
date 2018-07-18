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
 * @brief Set of utilities
 *
 * @file utils.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-13
 */

#ifndef __UTILS_H
#define __UTILS_H

#include <linux/list.h>

/**
 * @brief Create a list entry of the `type` specified and assign it to new and head (if head is
 * null)
 *
 * @param new pointer that will be assigned to the newly created entry
 * @param head (type list_head) pointer to the head of the list that will be assigned if it is null
 * @param type the type of the entry to create
 *
 */
#define create_list_entry(new, head, member, type)                                                 \
    new = kmalloc(sizeof(type), GFP_KERNEL);                                                       \
    mutex_lock(&fiber_lock);                                                                       \
    list_add_tail(&(new->member), head);                                                           \
    mutex_unlock(&fiber_lock);

/**
 * @brief Check if exist an entry (of type) in list pointed by head with field equal to value
 *
 * @param result pointer that will be assigned to the found entry, or NULL
 * @param head (type of list_head) pointer to the list head
 * @param field name of the field for checking the condition _equal to_ `value`
 * @param value value of the `field` for the condition
 * @param member name of the `struct list_head` inside the list entries
 * @param type name of the type of the entries
 *
 */
#define check_if_exists(result, head, field, value, member, type)                                  \
    mutex_lock(&fiber_lock);                                                                       \
    result = NULL;                                                                                 \
    if (!list_empty(head)) {                                                                       \
        type *type_temp__ = NULL;                                                                  \
        list_for_each_entry(type_temp__, head, member) {                                           \
            if (type_temp__->field == value) {                                                     \
                result = type_temp__;                                                              \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
    }                                                                                              \
    mutex_unlock(&fiber_lock);
#endif