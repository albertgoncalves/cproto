#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

static f32 euler(u32 n) {
    f32 m = 0.0f;
    f32 d = 1.0f;
    f32 e = 1.0f;
    for (u32 _ = 0; _ < n; ++_) {
        d *= ++m;
        e += 1.0f / d;
    }
    return e;
}

i32 main(void) {
    printf("%.5f\n", (f64)euler(10));
    return EXIT_SUCCESS;
}
