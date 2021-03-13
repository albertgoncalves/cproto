#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;

#define BURY_N(T, array, len, k, n)                \
    {                                              \
        usize start = len - k;                     \
        usize end = len - n;                       \
        T     copy[n];                             \
        for (usize i = 0; i < n; ++i) {            \
            copy[i] = array[end + i];              \
        }                                          \
        for (usize i = end - 1; start <= i; --i) { \
            array[i + n] = array[i];               \
        }                                          \
        for (usize i = 0; i < n; ++i) {            \
            array[start + i] = copy[i];            \
        }                                          \
    }

#define PRINT_ARRAY(fmt, array, len)      \
    {                                     \
        printf("[ ");                     \
        for (usize i = 0; i < len; ++i) { \
            printf(fmt " ", array[i]);    \
        }                                 \
        printf("]\n");                    \
    }

i32 main(void) {
    i32   array[] = {0, 1, 2, 3, 4, 5, 6};
    usize n = sizeof(array) / sizeof(array[0]);
    PRINT_ARRAY("%d", array, n);
    BURY_N(i32, array, n, 5, 2);
    PRINT_ARRAY("%d", array, n);
    return EXIT_SUCCESS;
}
