/**
 * @brief The header to be included by the user if it wants to use the library
 *
 * @file fiber.h
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-05-24
 */

#ifndef __FIBER_H
#define __FIBER_H

int ConvertThreadToFiber();
int CreateFiber(void *(*function)(void *), void *args);
int SwitchToFiber(unsigned fid);

#endif