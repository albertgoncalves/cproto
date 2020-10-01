#include <stdio.h>
#include <stdlib.h>

typedef size_t usize;

#define MIN(a, b) ((a) < (b) ? a : b)

static usize len(const char* x) {
    usize i = 0;
    while (x[i] != '\0') {
        ++i;
    }
    return i;
}

#define SWAP(T, a, b) \
    {                 \
        T t = a;      \
        a = b;        \
        b = t;        \
    }

static usize distance(const char* a, const char* b) {
    usize n = len(a);
    usize m = len(b);
    if (n == 0) {
        return m;
    }
    if (m == 0) {
        return n;
    }
    if (m < n) {
        SWAP(const char*, a, b);
        SWAP(usize, n, m);
    }
    usize* buffer = malloc(sizeof(usize) * (m + 1));
    for (usize j = 0; j < m; ++j) {
        buffer[j] = j;
    }
    for (usize i = 0; i < n; ++i) {
        usize previous = i + 1;
        for (usize j = 0; j < m; ++j) {
            usize penalty = a[i] != b[j];
            usize cost =
                MIN(MIN(previous + 1, buffer[j + 1] + 1), buffer[j] + penalty);
            buffer[j] = previous;
            previous = cost;
        }
        buffer[m] = previous;
    }
    usize result = buffer[m];
    free(buffer);
    return result;
}

#define TEST(a, b, value)          \
    if (distance(a, b) == value) { \
        printf(".");               \
    } else {                       \
        printf("!");               \
    }

int main(void) {
    TEST("foobar", "", 6);
    TEST("sitting", "kitten", 3);
    TEST("flaw", "lawn", 2);
    TEST("saturday", "sunday", 3);
    TEST("gumbo", "gambol", 2);
    TEST("book", "back", 2);
    TEST("edward", "edwin", 3);
    TEST("the quick brown fox jumps over the lazy dog",
         "pack my box with five dozen liquor jugs",
         33);
    printf("\n");
    return EXIT_SUCCESS;
}
