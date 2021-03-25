#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

i32 main(void) {
    u8 array8[] = {0, 1, 2, 3, 4, 5, 6, 7};
    {
        for (usize i = 0; i < 8; ++i) {
            printf("%-4hhu", array8[i]);
        }
        printf("\n");
    }
    {
        u16* array16 = (u16*)array8;
        for (usize i = 0; i < 4; ++i) {
            printf("%-8hu", array16[i]);
        }
        printf("\n");
    }
    {
        u32* array32 = (u32*)array8;
        for (usize i = 0; i < 2; ++i) {
            printf("%-16u", array32[i]);
        }
        printf("\n");
    }
    return EXIT_SUCCESS;
}
