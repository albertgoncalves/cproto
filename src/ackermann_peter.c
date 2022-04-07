#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;

static u32 ackermann(u32 m, u32 n) {
    if (m == 0) {
        return n + 1;
    } else if (n == 0) {
        return ackermann(m - 1, 1);
    } else {
        return ackermann(m - 1, ackermann(m, n - 1));
    }
}

i32 main(void) {
    for (u32 i = 0; i < 12; ++i) {
        printf("%u\n", ackermann(3, i));
    }
    return EXIT_SUCCESS;
}
