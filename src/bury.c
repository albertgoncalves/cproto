#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;

#define BURY(T, array, len, k)                \
    {                                         \
        usize m = len - 1;                    \
        T     x = array[m];                   \
        for (usize i = m - k; i < len; ++i) { \
            printf(".");                      \
            T t = array[i];                   \
            array[i] = x;                     \
            x = t;                            \
        }                                     \
        printf("\n");                         \
    }

#define BURY_N(T, array, len, k, n)                \
    {                                              \
        usize start = (len - 1) - k;               \
        usize end = len - n;                       \
        T     copy[n];                             \
        for (usize i = 0; i < n; ++i) {            \
            printf(".");                           \
            copy[i] = array[end + i];              \
        }                                          \
        for (usize i = end - 1; start <= i; --i) { \
            printf(".");                           \
            array[i + n] = array[i];               \
        }                                          \
        for (usize i = 0; i < n; ++i) {            \
            printf(".");                           \
            array[start + i] = copy[i];            \
        }                                          \
        printf("\n");                              \
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
    usize n = 5;
    usize k = 4;
    {
        i32   array[] = {0, 1, 2, 3, 4, 5, 6, 7};
        usize len = sizeof(array) / sizeof(array[0]);
        PRINT_ARRAY("%d", array, len);
        for (usize _ = 0; _ < k; ++_) {
            BURY(i32, array, len, n);
        }
        PRINT_ARRAY("%d", array, len);
    }
    printf("\n");
    {
        i32   array[] = {0, 1, 2, 3, 4, 5, 6, 7};
        usize len = sizeof(array) / sizeof(array[0]);
        PRINT_ARRAY("%d", array, len);
        BURY_N(i32, array, len, n, k);
        PRINT_ARRAY("%d", array, len);
    }
    return EXIT_SUCCESS;
}
