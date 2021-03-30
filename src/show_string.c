#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int32_t i32;
typedef uint8_t u8;
typedef size_t  usize;

typedef struct {
    usize       len;
    const char* chars;
} String;

#define PRINTLN_STR(string) printf("%.*s\n", (i32)string.len, string.chars)

#define STRING(literal)             \
    ((String){                      \
        .len = sizeof(literal) - 1, \
        .chars = literal,           \
    })

i32 main(void) {
    printf("sizeof(String) : %zu\n"
           "sizeof(\"Hello, world!\") : %zu\n",
           sizeof(String),
           sizeof("Hello, world!"));
    String string = STRING("Hello, world!");
    PRINTLN_STR(string);
    string.len = 5;
    PRINTLN_STR(string);
    return EXIT_SUCCESS;
}
