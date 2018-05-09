#include "fiber.h"

int main(int argc, char **argv) {
    printf("Starting fiber test!\n");
    ConvertThreadToFiber();
    exit(EXIT_SUCCESS);
}