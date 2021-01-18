#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

typedef float  f32;
typedef double f64;

#define U8_MAX  0xFF
#define U32_MAX 0xFFFFFFFF

typedef union {
    u32 u32;
    i32 i32;
    f32 f32;
} Union;

i32 main(void) {
    {
        u32 x = U32_MAX - 299999999;
        printf("\n%-12uu32\n%-12du32 -> i32\n", x, (i32)x);
    }
    {
        u8 x = U8_MAX - 79;
        printf("\n%-12uu32\n%-12du32 -> i32\n%-12du32 -> i8 -> i32\n",
               (u32)x,
               (i32)x,
               (i32)((i8)x));
    }
    {
        i16 x = -10;
        printf("\n%-12hdi16\n%-12hu*((u16*)(&...))\n", x, *((u16*)(&x)));
    }
    {
        Union x = {
            .u32 = 1077936128,
        };
        printf("\n%-12uu32\n%-12.1f*((f32*)(&...))\n", x.u32, (f64)x.f32);
    }
    {
        Union x = {
            .u32 = 1086324736,
        };
        printf("\n%-12uu32\n%-12.1f*((f32*)(&...))\n", x.u32, (f64)x.f32);
    }
    {
        Union x = {
            .f32 = -1.6f,
        };
        printf("\n%-12.1ff32\n%-12d*((i32*)(&...))\n", (f64)x.f32, x.i32);
    }
    {
        Union x = {
            .f32 = -1.0f,
        };
        printf("\n%-12.1ff32\n%-12d*((i32*)(&...))\n", (f64)x.f32, x.i32);
    }
    return EXIT_SUCCESS;
}
