#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef int64_t i64;

#define CODEGEN(A, B)                                                        \
    typedef struct Lazy_##A##_##B {                                          \
        B (*eval)(struct Lazy_##A##_##B*);                                   \
        union {                                                              \
            struct {                                                         \
                B (*func)(A);                                                \
                A args;                                                      \
            } closure;                                                       \
            B result;                                                        \
        } internal;                                                          \
    } Lazy_##A##_##B;                                                        \
                                                                             \
    static B lazy_from_cache_##A##_##B(Lazy_##A##_##B* lazy) {               \
        return lazy->internal.result;                                        \
    }                                                                        \
                                                                             \
    static B lazy_eval_##A##_##B(Lazy_##A##_##B* lazy) {                     \
        B result = lazy->internal.closure.func(lazy->internal.closure.args); \
        lazy->eval = lazy_from_cache_##A##_##B;                              \
        lazy->internal.result = result;                                      \
        return result;                                                       \
    }

typedef char* String;

CODEGEN(String, i64)

static i64 func(String message) {
    printf("%s\n", message);
    usleep(500000);
    return -123;
}

int main(void) {
    printf("sizeof(Lazy_String_i64): %zu\n", sizeof(Lazy_String_i64));

    Lazy_String_i64 lazy = (Lazy_String_i64){
        .eval = lazy_eval_String_i64,
        .internal = {.closure = {.func = func, .args = "?!"}},
    };

    for (int i = 0; i < 3; ++i) {
        printf("%ld\n", lazy.eval(&lazy));
    }

    return 0;
}
