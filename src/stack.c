#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int64_t i64;
typedef size_t  usize;

#define CONCAT(a, b) a##b

#define CODEGEN_STACK(T)                                                          \
    typedef struct {                                                              \
        T*    items;                                                              \
        usize len;                                                                \
        usize cap;                                                                \
    } CONCAT(Stack_, T);                                                          \
                                                                                  \
    static void CONCAT(push_, T)(CONCAT(Stack_, T) * stack, T x) {                \
        if (stack->len == stack->cap) {                                           \
            stack->cap = ((stack->cap | 1lu) + 7lu) & ~7lu;                       \
            stack->items = (T*)reallocarray(stack->items, stack->cap, sizeof(T)); \
            assert(stack->items);                                                 \
        }                                                                         \
        stack->items[stack->len++] = x;                                           \
    }                                                                             \
                                                                                  \
    static T CONCAT(pop_, T)(CONCAT(Stack_, T) * stack) {                         \
        assert(stack->len);                                                       \
        return stack->items[--stack->len];                                        \
    }

CODEGEN_STACK(i64)

int main(void) {
    Stack_i64 stack = {};

    for (i64 i = 0; i < 10; ++i) {
        push_i64(&stack, i);
        printf("len: %2zu, cap: %2zu\n", stack.len, stack.cap);
    }

    while (stack.len != 0) {
        printf("%ld\n", pop_i64(&stack));
    }

    free(stack.items);
    return 0;
}
