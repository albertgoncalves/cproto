#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;

u32 ackermann(u32, u32);
u32 ackermann(u32 m, u32 n) {
    if (m == 0) {
        return n + 1;
    } else if (n == 0) {
        return ackermann(m - 1, 1);
    } else {
        return ackermann(m - 1, ackermann(m, n - 1));
    }
}

i32 main(void) {
    printf("%u\n", ackermann(3, 9));
    return EXIT_SUCCESS;
}
