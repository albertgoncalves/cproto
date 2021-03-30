#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;

typedef void (*Fn)(void);

static void a(void) {
    printf("a\n");
}

static void b(void) {
    printf("b\n");
}

static void c(void) {
    printf("c\n");
}

i32 main(void) {
    Fn    fns[] = {a, b, c};
    usize n = sizeof(fns) / sizeof(fns[0]);
    for (usize i = 0; i < n; ++i) {
        fns[i]();
    }
    return EXIT_SUCCESS;
}
