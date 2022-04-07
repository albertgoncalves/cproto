#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int32_t i32;

#define SWAP(array, i, j)    \
    {                        \
        u8 t = array[i];     \
        array[i] = array[j]; \
        array[j] = t;        \
    }

static i32 partition(u8* array, i32 low, i32 high) {
    u8  pivot = array[high];
    i32 i = low - 1;
    for (i32 j = low; j < high; ++j) {
        if (array[j] <= pivot) {
            ++i;
            SWAP(array, i, j);
        }
    }
    ++i;
    SWAP(array, i, high);
    return i;
}

#undef SWAP

static void quicksort(u8* array, i32 low, i32 high) {
    if (low < high) {
        i32 i = partition(array, low, high);
        if (i != 0) {
            quicksort(array, low, i - 1);
        }
        quicksort(array, i + 1, high);
    }
}

static void show(u8* array, i32 n) {
    printf("[ ");
    for (i32 i = 0; i < n; ++i) {
        printf("%hhu ", array[i]);
    }
    printf("]\n");
}

i32 main(void) {
    u8  array[] = {7, 0, 9, 3, 2, 4, 7, 1, 1, 0, 5, 2, 8, 9, 6, 3, 3, 2, 2};
    i32 n = sizeof(array) / sizeof(array[0]);
    show(array, n);
    quicksort(array, 0, n - 1);
    show(array, n);
    return EXIT_SUCCESS;
}
