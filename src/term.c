#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned char u8;

#define U8_MAX 0xFF
#define N      (u8)10

int main(void) {
    for (u8 i = 0;; ++i) {
        for (u8 _ = 0; _ < N; ++_) {
            printf("%8hhu\n", i);
        }
        for (u8 _ = 0; _ < N; ++_) {
            printf(" ");
        }
        if (U8_MAX <= i) {
            break;
        }
        printf("\033[%hhuA\033[%hhuD", N, N);
        usleep(5000);
    }
    return EXIT_SUCCESS;
}
