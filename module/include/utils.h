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
    list_add_tail(&(new->member), head);

/**
 * @brief Create a list entry of the `type` specified and assign it to new
 *
 */
#define create_hash_entry(new, type, hashtable, node, key)                                         \
    new = kmalloc(sizeof(type), GFP_KERNEL);                                                       \
    hash_add(hashtable, node, key);

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
    result = NULL;                                                                                 \
    if (!list_empty(head)) {                                                                       \
        type *type_temp__ = NULL;                                                                  \
        type *type_temp_safe__ = NULL;                                                             \
        list_for_each_entry_safe(type_temp__, type_temp_safe__, head, member) {                    \
            if (type_temp__->field == value) {                                                     \
                result = type_temp__;                                                              \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
    }

/**
 * @brief Check if a node exists in the hash table
 *
 * @param result pointer that will be assigned to the found entry, or NULL
 * @param hashtable pointer to the hashtable
 * @param field name of the field for checking the condition _equal to_ `value`
 * @param value value of the `field` for the condition
 * @param member name of the `struct hlist_node` inside the entries
 * @param type name of the type of the entries
 */
#define check_if_exists_hash(result, hashtable, field, value, member, type)                        \
    result = NULL;                                                                                 \
    if (!hash_empty(hashtable)) {                                                                  \
        type *__cursor_temp_hash = NULL;                                                           \
        struct hlist_node *__cursor_temp_hash_safe = NULL;                                         \
        hash_for_each_possible_safe(hashtable, __cursor_temp_hash, __cursor_temp_hash_safe,        \
                                    member, value) {                                               \
            if (__cursor_temp_hash->field == value) {                                              \
                result = __cursor_temp_hash;                                                       \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
    }

#endif