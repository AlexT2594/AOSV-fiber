
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

#include "list.h"

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
    new = (type *)malloc(sizeof(type));                                                            \
    list_add_tail(&(new->member), head);

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
        list_for_each_entry(type_temp__, head, member) {                                           \
            if (type_temp__->field == value) {                                                     \
                result = type_temp__;                                                              \
                break;                                                                             \
            }                                                                                      \
        }                                                                                          \
    }
#endif