#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef size_t  usize;

static void reverse(u8* array, usize size) {
    usize m = size >> 1;
    for (usize i = 0; i < m; ++i) {
        usize j = size - i - 1;
        u8 t = array[i];
        array[i] = array[j];
        array[j] = t;
    }
}

#define N 11

int main(void) {
    u8 array[N] = {0};
    for (usize i = 0; i < N; ++i) {
        array[i] = (u8)i;
    }
    reverse(array, N);
    printf("[");
    for (usize i = 0; i < N; ++i) {
        printf(" %hhu", array[i]);
    }
    printf(" ]\n");
    return EXIT_SUCCESS;
}
