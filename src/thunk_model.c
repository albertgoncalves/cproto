#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t  i32;
typedef int64_t  i64;
typedef uint64_t u64;

typedef void* (*Func1)(void*);

typedef struct {
    union {
        Func1 func;
        void* result;
    } head;
    u64   info;
    void* arg;
} Thunk1;

static Thunk1* alloc_thunk1(Func1 func, void* arg) {
    Thunk1* thunk = (Thunk1*)malloc(sizeof(Thunk1));
    thunk->head.func = func;
    thunk->info = 0;
    thunk->arg = arg;
    return thunk;
}

static void* call_thunk1(Thunk1* thunk) {
    if (thunk->info & 1) {
        return thunk->head.result;
    }

    void* result = thunk->head.func(thunk->arg);
    thunk->head.result = result;
    thunk->info |= 1;
    return result;
}

static void* f1(void* x) {
    printf("f1\n");
    return (void*)(((i64)x) + 1);
}

i32 main(void) {
    Thunk1* thunk = alloc_thunk1(f1, (void*)-123);

    for (i32 i = 0; i < 3; ++i) {
        void* result = call_thunk1(thunk);
        printf("%ld\n", (i64)result);
    }

    return 0;
}
