#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;
typedef size_t   usize;

static u32 MEMORY[1 << 14];

static u32 ackermann_peter(u32 m, u32 n) {
    u32*  stack = MEMORY;
    usize len = 0;

    for (;;) {
        if (m == 0) {
            n += 1;
            if (len == 0) {
                return n;
            }
            m = stack[--len];
            continue;
        }

        if (n == 0) {
            m -= 1;
            n = 1;
            continue;
        }

        stack[len++] = m - 1;
        n -= 1;
    }
}

int main(void) {
    printf("%u\n", ackermann_peter(3, 10));
    return 0;
}
