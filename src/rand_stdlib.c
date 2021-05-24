#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef float   f32;
typedef double  f64;

static f32 randf(void) {
    i32 x = rand();
    return (f32)x / (f32)RAND_MAX;
}

i32 main(void) {
    srand(10u);
    f32 a = randf();
    f32 b = randf();
    printf("%d\n%.6f\n%.6f\n", RAND_MAX, (f64)a, (f64)b);
    return EXIT_SUCCESS;
}
