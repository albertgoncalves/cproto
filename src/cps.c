#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef int32_t i32;
typedef int64_t i64;
typedef size_t  usize;

typedef union Word Word;

typedef void (*Func)(i64);

union Word {
    Func as_func;
    i64  as_i64;
};

STATIC_ASSERT(sizeof(void*) == 8);
STATIC_ASSERT(sizeof(void*) == sizeof(Word));

#define STACK_CAP (1 << 14)
static Word  STACK[STACK_CAP] = {0};
static usize STACK_LEN = 0;

static Word from_func(Func func) {
    return (Word){.as_func = func};
}

static Word from_i64(i64 x) {
    return (Word){.as_i64 = x};
}

static void push_stack(Word word) {
    assert(STACK_LEN < STACK_CAP);
    STACK[STACK_LEN++] = word;
}

static Word pop_stack(void) {
    assert(STACK_LEN != 0);
    return STACK[--STACK_LEN];
}

static void ackermann_peter_cps_inner(i64);

static void ackermann_peter_cps(Func func, i64 m, i64 n) {
    if (m == 0) {
        func(n + 1);
    } else if (n == 0) {
        ackermann_peter_cps(func, m - 1, 1);
    } else {
        push_stack(from_i64(m));
        push_stack(from_func(func));
        ackermann_peter_cps(&ackermann_peter_cps_inner, m, n - 1);
    }
}

static void ackermann_peter_cps_inner(i64 n) {
    Func      func = pop_stack().as_func;
    const i64 m = pop_stack().as_i64;
    ackermann_peter_cps(func, m - 1, n);
}

static void print(i64 x) {
    printf("%ld\n", x);
}

i32 main(void) {
    const i64 m = 3;
    for (i64 n = 0; n < 11; ++n) {
        ackermann_peter_cps(&print, m, n);
    }
}
