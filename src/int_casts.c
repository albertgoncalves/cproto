#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int32_t i32;

#define U8_MAX  0xFF
#define U32_MAX 0xFFFFFFFF

i32 main(void) {
    {
        u32 x = U32_MAX - 299999999;
        printf("%-12uu32\n%-12du32 -> i32\n", x, (i32)x);
    }
    {
        u8 x = U8_MAX - 79;
        printf("%-12uu32\n%-12du32 -> i32\n%-12du32 -> i8 -> i32\n",
               (u32)x,
               (i32)x,
               (i32)((i8)x));
    }
    return EXIT_SUCCESS;
}
