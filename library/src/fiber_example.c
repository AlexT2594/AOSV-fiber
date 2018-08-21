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
 * @brief This file represent a test file
 *
 * @file fiber_example.c
 * @author Gabriele Proietti Mattia <gabry.gabry@hotmail.it>
 * @author Alexandru Daniel Tufa <alex.tufa94@gmail.com>
 * @date 2018-07-18
 */

#include "fiber.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void hello(void *args);
void f1(void *args);
void f11(void *args);

int main_fiber, thread1_fiber, fiber1, fiber2;

int main(int argc, char **argv) {
    printf("Starting fiber test!\n");
    main_fiber = ConvertThreadToFiber();
    long index = FlsAlloc();
    printf("Index is %ld\n", index);
    long index2 = FlsAlloc();
    printf("Index2 is %ld\n", index2);
    // test set
    printf("Ret value of FlsSetValue() is %d\n", FlsSetValue(index, 34));
    printf("Ret value of FlsSetValue() is %d\n", FlsSetValue(32, 34));

    long value = FlsGetValue(0);
    printf("Value of index 0 is %ld\n", value);

    // int a = 5;
    // fiber1 = CreateFiber(STACK_SIZE, (void *)&hello, &a);
    // printf("Main fiber fid is %d\n", main_fiber);
    // SwitchToFiber(my_new_fiber);
    // ConvertThreadToFiber();
    // pthread_t t1;
    // pthread_t t2;
    // pthread_create(&t1, NULL, (void *)f1, NULL);
    // sleep(2);
    // pthread_create(&t2, NULL, ConvertThreadToFiber, NULL);
    // pthread_join(t1, NULL);
    // SwitchToFiber(fiber1);
    // pthread_join(t2, NULL);
    // CreateFiber(&hello);
    exit(EXIT_SUCCESS);
}

void hello(void *args) {
    int a = 3;
    printf("Hello called\n");
    printf("a is %d\n", a);
    // exit(EXIT_SUCCESS);
}

void f11(void *args) {
    printf("f11 called\n");
    SwitchToFiber(fiber1);
}

void f1(void *args) {
    thread1_fiber = ConvertThreadToFiber();
    int fiber2 = CreateFiber(STACK_SIZE, (void *)&f11, NULL);
    SwitchToFiber(fiber2);
    printf("Returned from main_fiber, will not call Switch intentionally! \n");
}
