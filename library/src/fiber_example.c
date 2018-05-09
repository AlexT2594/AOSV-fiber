#include "fiber.h"
#include <pthread.h>

int hello();

int main(int argc, char **argv) {
    printf("Starting fiber test!\n");
    ConvertThreadToFiber();
    ConvertThreadToFiber();
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

int hello() { printf("Hello called"); }