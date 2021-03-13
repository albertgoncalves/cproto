#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;

typedef enum {
    ENUM_A = 0,
    ENUM_B,
    ENUM_C,
    ENUM_D,
    ENUM_COUNT,
} Enum;

static const i32 ENUM_INFO[ENUM_COUNT] = {
    [ENUM_A] = 3,
    [ENUM_B] = 2,
    [ENUM_D] = -1,
};

#define PRINT_ARRAY(fmt, array, n)      \
    {                                   \
        printf("[ ");                   \
        for (usize i = 0; i < n; ++i) { \
            printf(fmt " ", array[i]);  \
        }                               \
        printf("]\n");                  \
    }

i32 main(void) {
    PRINT_ARRAY("%d", ENUM_INFO, ENUM_COUNT);
    return EXIT_SUCCESS;
}
