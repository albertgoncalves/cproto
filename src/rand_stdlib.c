#include <stdio.h>
#include <stdlib.h>

typedef float f32;

static f32 randf(void) {
    return (f32)rand() / (f32)RAND_MAX;
}

int main(void) {
    srand(10u);
    f32 a = randf();
    f32 b = randf();
    printf("%d\n %.6f\n%.6f\n", RAND_MAX, (double)a, (double)b);
    return EXIT_SUCCESS;
}
