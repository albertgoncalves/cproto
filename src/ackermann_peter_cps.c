#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;

typedef int64_t i64;

typedef void (*Func)(u32);

typedef struct {
    Func func;
    u32  m;
} Thunk;

static Thunk  MEMORY[1 << 13];
static Thunk* STACK = MEMORY;

static void inner(u32);

static void ackermann_peter(Func func, u32 m, u32 n) {
    if (m == 0) {
        func(n + 1);
    } else if (n == 0) {
        ackermann_peter(func, m - 1, 1);
    } else {
        *(STACK++) = (Thunk){.func = func, .m = m - 1};
        ackermann_peter(inner, m, n - 1);
    }
}

void inner(u32 n) {
    const Thunk thunk = *(--STACK);
    ackermann_peter(thunk.func, thunk.m, n);
}

static void print(u32 n) {
    printf("%u\n", n);
}

int main(void) {
    ackermann_peter(print, 3, 10);
    return 0;
}
