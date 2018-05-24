#include "fiber.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void hello(void *args);

int main(int argc, char **argv) {
    printf("Starting fiber test!\n");
    ConvertThreadToFiber();
    int a = 5;
    int my_new_fiber = CreateFiber((void *)&hello, &a);
    printf("My new fid is %d\n", my_new_fiber);
    SwitchToFiber(my_new_fiber);
    // ConvertThreadToFiber();
    // pthread_t t1;
    // pthread_t t2;
    // pthread_create(&t1, NULL, ConvertThreadToFiber, NULL);
    // sleep(2);
    // pthread_create(&t2, NULL, ConvertThreadToFiber, NULL);
    // pthread_join(t1, NULL);
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