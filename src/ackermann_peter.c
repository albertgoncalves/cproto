#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;
typedef int32_t  i32;

#define OK 0

static u32 ackermann_peter(u32 m, u32 n) {
    if (m == 0) {
        return n + 1;
    } else if (n == 0) {
        return ackermann_peter(m - 1, 1);
    } else {
        return ackermann_peter(m - 1, ackermann_peter(m, n - 1));
    }
}

i32 main(void) {
    for (u32 i = 0; i < 11; ++i) {
        printf("%u\n", ackermann_peter(3, i));
    }
    return OK;
}
