#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;

typedef int32_t i32;

#define OK 0

#define N 9

i32 main(void) {
    u32 array[N];

    for (u32 i = 0; i < N; ++i) {
        array[i] = i + 1;
    }

    for (u32 i = 1; (i * 2) < N; ++i) {
        const u32 x = array[i];
        array[i] = array[i * 2];
        array[i * 2] = x;
    }

    for (u32 i = 0;;) {
        printf("%u", array[i++]);
        if (N <= i) {
            putchar('\n');
            break;
        }
        putchar(' ');
    }

    return OK;
}
