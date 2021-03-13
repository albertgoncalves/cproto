#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

typedef struct {
    u8 x;
    u8 y;
    u8 z;
} T;

i32 main(void) {
    printf("alignof(char)        : %zu\n"
           "alignof(u16)         : %zu\n"
           "alignof(i32)         : %zu\n"
           "alignof(f32)         : %zu\n"
           "alignof(void*)       : %zu\n"
           "alignof(f64)         : %zu\n"
           "alignof(max_align_t) : %zu\n"
           "alignof(T)           : %zu\n"
           "sizeof(T)            : %zu\n",
           alignof(char),
           alignof(u16),
           alignof(i32),
           alignof(f32),
           alignof(f64),
           alignof(void*),
           alignof(max_align_t),
           alignof(T),
           sizeof(T));
    return EXIT_SUCCESS;
}
