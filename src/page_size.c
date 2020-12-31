#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef int32_t i32;

i32 main(void) {
    printf("%d\n", getpagesize());
    return EXIT_SUCCESS;
}
