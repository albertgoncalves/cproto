#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t  i32;
typedef uint32_t u32;

i32 main(void) {
    {
        u32 x = 0xFFFFFFFF;
        u32 y = 50;
        printf("%u\n", x - y);
        printf("%d\n", (i32)(x - y));
    }
    {
        i32 x = -1;
        i32 y = 50;
        printf("%u\n", (u32)(x - y));
        printf("%d\n", x - y);
    }
    {
        i32 x = 50;
        printf("%d\n", !!x);
    }
    return EXIT_SUCCESS;
}
