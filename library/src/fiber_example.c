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
#include <unistd.h>

void fnA(void *args);
void fnB(void *args);
void fnC(void *args);
void fnD(void *args);

int main_fiber, thread1_fiber, fiber1, fiber2;

int main(int argc, char **argv) {
    printf("Starting fiber test!\n");
    main_fiber = ConvertThreadToFiber();
    long index = FlsAlloc();
    printf("FlsAlloc() got %ld\n", index);
    long index2 = FlsAlloc();
    printf("FlsAlloc() got %ld\n", index2);
    // test set
    unsigned long n = 1400708332293323492;
    printf("Ret value of FlsSetValue(index,34) is %d\n", FlsSetValue(index, n));
    // printf("Ret value of FlsSetValue(32,34) is %d\n", FlsSetValue(32, 34));

    long value = FlsGetValue(0);
    printf("Value of index 0 is %lu\n", value);
    value = FlsGetValue(0);
    printf("Value of index 0 is %lu\n", value);
    value = FlsGetValue(0);
    printf("Value of index 0 is %lu\n", value);

    int a = 5;
    fiber1 = CreateFiber(STACK_SIZE, (void *)&fnA, &a);
    fiber2 = CreateFiber(STACK_SIZE, (void *)&fnB, NULL);
    printf("Main fiber fid is %d\n", main_fiber);
    // SwitchToFiber(fiber1);

    // printf("Returned from fiber1, can continue\n");

    // SwitchToFiber(fiber2);
    // SwitchToFiber(my_new_fiber);
    // ConvertThreadToFiber();
    pthread_t t1;
    // pthread_t t2;
    printf("Creating thread..");
    pthread_create(&t1, NULL, (void *)fnC, NULL);
    sleep(60);
    // pthread_create(&t2, NULL, ConvertThreadToFiber, NULL);
    // pthread_join(t1, NULL);
    // pthread_join(t2, NULL);
    // CreateFiber(&hello);
    exit(EXIT_SUCCESS);
}

void fnA(void *args) {
    // int a = 3;
    printf("fnA()\n");
    SwitchToFiber(0);
    // printf("a is %d\n", a);
}

void fnB(void *args) {
    printf("fnB()\n");
    SwitchToFiber(0);
}

void fnC(void *args) {
    printf("I`m in the thread\n");
    thread1_fiber = ConvertThreadToFiber();
    int fiber2 = CreateFiber(STACK_SIZE, (void *)&fnD, NULL);
    if (fiber2 < 0) {
        printf("ERROR Im not a fiber!");
        exit(EXIT_FAILURE);
    }
    SwitchToFiber(fiber2);
    printf("Returned from main_fiber, will not call Switch intentionally! \n");
}

void fnD(void *args) {
    printf("fnD()\n");
    // SwitchToFiber(0);
}