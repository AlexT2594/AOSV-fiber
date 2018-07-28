/**
 * Copyright (C) 2018 Gabriele Proietti Mattia <gabry.gabry@hotmail.it> & Alexandru Daniel Tufa
 * <alex.tufa94@gmail.com>
 *
 * This file is part of Fibers (Library).
 *
 * Fibers (Library) is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fibers (Library) is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fibers (Library).  If not, see <http://www.gnu.org/licenses/>.
 *
 */

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

#define STACK_SIZE 2048 * 8

int ConvertThreadToFiber();
int CreateFiber(unsigned long stack_size, void *(*function)(void *), void *args);
int SwitchToFiber(unsigned fid);

#endif