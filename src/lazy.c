#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef uint64_t u64;

typedef struct {
    void* (*force)(void*);
    union {
        void* value;
        void* (*func)(void*);
    } _;
    void* args;
} Lazy;

STATIC_ASSERT(sizeof(void*) == sizeof(u64));

static void* lazy_cache(void* object) {
    printf(".\n");
    return ((Lazy*)object)->_.value;
}

static void* lazy_force(void* object) {
    printf("!\n");
    void* value = ((Lazy*)object)->_.func(((Lazy*)object)->args);
    ((Lazy*)object)->force = lazy_cache;
    ((Lazy*)object)->_.value = value;
    return value;
}

static Lazy* lazy_new(void* (*func)(void*), void* args) {
    Lazy* object = (Lazy*)calloc(1, sizeof(Lazy));
    object->_.func = func;
    object->args = args;
    object->force = lazy_force;
    return object;
}

static void* work(void*) {
    usleep(500000);
    return (void*)123;
}

int main(void) {
    printf("sizeof(Lazy): %zu\n", sizeof(Lazy));
    Lazy* object = lazy_new(work, NULL);
    object->force(object);
    object->force(object);
    printf("%lu\n", (u64)(object->force(object)));
    free(object);
    return 0;
}
