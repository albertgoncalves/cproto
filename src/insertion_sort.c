#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int32_t i32;

static void insertion_sort_asc(u8* array, u8 n) {
    for (u8 i = 0; i < n; ++i) {
        for (u8 j = i; (0 < j) && (array[j] < array[j - 1]); --j) {
            u8 t = array[j];
            array[j] = array[j - 1];
            array[j - 1] = t;
        }
    }
}

static void print(u8* array, u8 n) {
    printf("[");
    for (u8 i = 0; i < n; ++i) {
        printf(" %hhu", array[i]);
    }
    printf(" ]\n");
}

i32 main(void) {
    u8 array[] = {0, 5, 3, 6, 4, 7, 8, 1, 0};
    u8 n = sizeof(array) / sizeof(array[0]);
    print(array, n);
    insertion_sort_asc(array, n);
    print(array, n);
    return EXIT_SUCCESS;
}
