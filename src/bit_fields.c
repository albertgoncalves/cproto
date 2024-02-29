#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;

typedef uint64_t u64;

#define OK 0

typedef union {
    struct {
        u64 bit : 1;
        u64 _ : 63;
    } as_top;
    struct {
        u64 _ : 63;
        u64 bit : 1;
    } as_bottom;
    void* as_pointer;
} Box;

i32 main(void) {
    Box top = {.as_top = {.bit = 1}};
    Box bottom = {.as_bottom = {.bit = 1}};
    printf("top    : %p\n"
           "bottom : %p\n",
           top.as_pointer,
           bottom.as_pointer);
    return OK;
}
