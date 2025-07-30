#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef int64_t i64;

typedef struct {
    void* (*func)(void*);
    void* args;
} Closure;

typedef struct Lazy Lazy;

struct Lazy {
    void* (*eval)(Lazy*);
    Closure closure;
};

static void* lazy_from_cache(Lazy* lazy) {
    return lazy->closure.args;
}

static void* lazy_eval(Lazy* lazy) {
    void* result = lazy->closure.func(lazy->closure.args);
    lazy->eval = lazy_from_cache;
    lazy->closure.args = result;
    return result;
}

static void* func(void*) {
    usleep(500000);
    return (void*)-123;
}

int main(void) {
    printf("sizeof(Closure) : %zu\n"
           "sizeof(Lazy)    : %zu\n"
           "\n",
           sizeof(Closure),
           sizeof(Lazy));

    Lazy lazy = (Lazy){
        .eval = lazy_eval,
        .closure = {.func = func, .args = NULL},
    };

    for (int i = 0; i < 3; ++i) {
        void* result = lazy.eval(&lazy);
        printf("%ld\n", (i64)result);
    }

    return 0;
}
