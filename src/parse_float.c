#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;
typedef float   f32;
typedef double  f64;

static f32 parse_f32(const char* p) {
    f32 x = 0.0f;
    while (('0' <= *p) && (*p <= '9')) {
        x = (x * 10.0f) + ((f32)(*(p++) - '0'));
    }
    if (*(p++) == '.') {
        f32 a = 0.0f;
        f32 b = 1.0f;
        while (('0' <= *p) && (*p <= '9')) {
            a = (a * 10.0f) + ((f32)(*(p++) - '0'));
            b *= 10.0f;
        }
        x += a / b;
    }
    return x;
}

i32 main(void) {
    printf("%f\n%f\n", (f64)parse_f32("9.5"), (f64)parse_f32("8.321"));
    return EXIT_SUCCESS;
}
