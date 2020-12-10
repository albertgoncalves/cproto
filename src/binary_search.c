#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t i32;
typedef size_t  usize;

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

typedef struct {
    usize value;
    Bool  ok;
} Result;

static Result binary_search(i32* array, usize left, usize right, i32 target) {
    Result result = {0};
    usize  l = left;
    usize  r = right;
    for (;;) {
        if (r < l) {
            return result;
        }
        usize i = l + ((r - l) / 2);
        i32   value = array[i];
        if (value == target) {
            result.ok = TRUE;
            result.value = i;
            return result;
        } else if (target < value) {
            r = i - 1;
        } else {
            l = i + 1;
        }
    }
}

i32 main(void) {
    printf("sizeof(Bool)   : %zu\n"
           "sizeof(usize)  : %zu\n"
           "sizeof(Result) : %zu\n",
           sizeof(Bool),
           sizeof(usize),
           sizeof(Result));
    i32   array[] = {-5, -4, -2, 0, 1, 4, 10, 11, 12, 13, 14, 17, 20, 24, 36};
    usize n = sizeof(array) / sizeof(array[0]);
    for (usize i = 0; i < n; ++i) {
        Result result = binary_search(array, 0, n, array[i]);
        if (result.ok) {
            printf("%zu\n", result.value);
        }
    }
    return EXIT_SUCCESS;
}
