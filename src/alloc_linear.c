#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define CAP_MEMORY_BUFFER 32

typedef struct {
    usize len;
    u8    buffer[CAP_MEMORY_BUFFER];
} Memory;

typedef struct {
    u8   len;
    char chars[];
} String;

#define PRINT_STRING(string) printf("%.*s\n", (i32)string->len, string->chars)

static String* alloc_string(Memory* memory, u8 len, const char* chars) {
    usize start = memory->len;
    usize gap = start % alignof(String);
    if (gap != 0) {
        start += alignof(String) - gap;
    }
    usize end = start + sizeof(String) + len;
    if (CAP_MEMORY_BUFFER <= end) {
        return NULL;
    }
    String* string = (String*)&(memory->buffer[start]);
    string->len = len;
    memory->len = end;
    memcpy(&string->chars, chars, len);
    return string;
}

i32 main(void) {
    Memory* memory = calloc(1, sizeof(Memory));
    String* x = alloc_string(memory, 13, "Hello, world!");
    String* y = alloc_string(memory, 8, "Goodbye?");
    String* z = alloc_string(memory, 3, "Ok.");
    {
        printf("[ ");
        for (usize i = 0; i < CAP_MEMORY_BUFFER; ++i) {
            printf("%hhu ", memory->buffer[i]);
        }
        printf("]\n");
    }
    PRINT_STRING(x);
    PRINT_STRING(y);
    PRINT_STRING(z);
    free(memory);
    return EXIT_SUCCESS;
}
