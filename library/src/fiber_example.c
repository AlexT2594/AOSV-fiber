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
    int a = 5;
    fiber1 = CreateFiber((void *)&hello, &a);
    printf("Main fiber fid is %d\n", main_fiber);
    // SwitchToFiber(my_new_fiber);
    // ConvertThreadToFiber();
    pthread_t t1;
    // pthread_t t2;
    pthread_create(&t1, NULL, (void *)f1, NULL);
    // sleep(2);
    // pthread_create(&t2, NULL, ConvertThreadToFiber, NULL);
    pthread_join(t1, NULL);
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
    int fiber2 = CreateFiber((void *)&f11, NULL);
    SwitchToFiber(fiber2);
    printf("Returned from main_fiber, will not call Switch intentionally! \n");
}
