#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int32_t i32;

typedef struct {
    const char* string;
    u8          start;
    u8          len;
} Slice;

#define PRINT_VIEW(view) \
    printf("\"%.*s\"\n", view.len, view.string + view.start)

static const char* STRING = "foo bar baz";

i32 main(void) {
    printf("sizeof(Slice) : %zu\n", sizeof(Slice));
    Slice view = {
        .string = STRING,
        .start = 3,
        .len = 5,
    };
    PRINT_VIEW(view);
    return EXIT_SUCCESS;
}
