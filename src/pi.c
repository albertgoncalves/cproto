#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

static f64 pi(u32 n) {
    f64 x = 1.0;
    f64 d = 1.0;
    for (u32 _ = 0; _ < n; ++_) {
        d += 2.0;
        x -= 1.0 / d;
        d += 2.0;
        x += 1.0 / d;
    }
    return x * 4.0;
}

i32 main(void) {
    printf("%.7f\n", pi(10000000));
    return EXIT_SUCCESS;
}
