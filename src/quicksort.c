#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef size_t  usize;

#define SWAP(array, i, j)    \
    {                        \
        u8 t = array[i];     \
        array[i] = array[j]; \
        array[j] = t;        \
    }

static usize partition(u8* array, usize low, usize high) {
    u8    pivot = array[high];
    usize i = low - 1;
    for (usize j = low; j < high; ++j) {
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

void quicksort(u8*, usize, usize);
void quicksort(u8* array, usize low, usize high) {
    if (low < high) {
        usize i = partition(array, low, high);
        if (i != 0) {
            quicksort(array, low, i - 1);
        }
        quicksort(array, i + 1, high);
    }
}

static void show(u8* array, usize n) {
    printf("[ ");
    for (usize i = 0; i < n; ++i) {
        printf("%hhu ", array[i]);
    }
    printf("]\n");
}

i32 main(void) {
    u8    array[] = {7, 0, 9, 3, 2, 4, 7, 1, 1, 0, 5, 2, 8, 9, 6, 3, 3, 2, 2};
    usize n = sizeof(array) / sizeof(array[0]);
    show(array, n);
    quicksort(array, 0, n - 1);
    show(array, n);
    return EXIT_SUCCESS;
}
