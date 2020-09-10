#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int8_t  i8;

void merge_sort(i8*, i8*, i8*, u8, u8);

void merge_sort(i8* array, i8* a, i8* b, u8 l, u8 r) {
    if (l < r) {
        u8 m = (u8)(l + ((r - l) / (u8)2u));
        merge_sort(array, a, b, l, m);
        merge_sort(array, a, b, (u8)(m + 1), r);
        u8 m1 = (u8)(m + 1);
        u8 n1 = (u8)((m - l) + 1);
        u8 n2 = (u8)(r - m);
        for (u8 i = 0; i < n1; ++i) {
            a[i] = array[l + i];
        }
        for (u8 i = 0; i < n2; ++i) {
            b[i] = array[m1 + i];
        }
        u8 i = 0;
        u8 j = 0;
        u8 k = l;
        while ((i < n1) && (j < n2)) {
            if (a[i] <= b[j]) {
                array[k] = a[i];
                ++i;
            } else {
                array[k] = b[j];
                ++j;
            }
            ++k;
        }
        while (i < n1) {
            array[k] = a[i];
            ++i;
            ++k;
        }
        while (j < n2) {
            array[k] = b[j];
            ++j;
            ++k;
        }
    }
}

int main(void) {
    i8  array[] = {12, 11, 5, -1, 13, 6, 7};
    u8  n = (u8)(sizeof(array) / sizeof(array[0]));
    u8  m = (u8)(n - 1);
    i8* buffer = malloc(sizeof(u8) * n * 2u);
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }
    i8* a = buffer;
    i8* b = &buffer[n];
    merge_sort(array, a, b, 0, m);
    free(buffer);
    printf("[");
    for (u8 i = 0u; i < m; ++i) {
        printf("%hhd, ", array[i]);
    }
    printf("%hhd]\n", array[m]);
    return EXIT_SUCCESS;
}
