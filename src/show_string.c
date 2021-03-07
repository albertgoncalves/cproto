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

static String get_string(const char* chars) {
    return (String){
        .len = strlen(chars),
        .chars = chars,
    };
}

i32 main(void) {
    printf("sizeof(String) : %zu\n", sizeof(String));
    const char* chars = "Hello, world!";
    String      string = get_string(chars);
    string.len = 5;
    PRINTLN_STR(string);
    return EXIT_SUCCESS;
}
