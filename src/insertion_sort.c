#include <stdio.h>

static void insertion_sort(int* array, int n) {
    for (int i = 0; i < n; ++i) {
        const int x = array[i];

        int j = i;
        for (; (0 < j) && (x < array[j - 1]); --j) {
            array[j] = array[j - 1];
        }
        array[j] = x;
    }
}

static void print(int* array, int n) {
    for (int i = 0; i < n; ++i) {
        printf(" %2hhu", array[i]);
    }
    putchar('\n');
}

int main(void) {
    int array[] = {14, 13, 0, 5, 3, 12, 6, 4, 9, 2, 7, 8, 1, 11, 10};

#define N (sizeof(array) / sizeof(array[0]))
    print(array, N);
    insertion_sort(array, N);
    print(array, N);

    return 0;
}
