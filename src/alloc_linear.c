#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define CAP_MEMORY_BUFFER 64

typedef struct {
    usize len;
    u8    buffer[CAP_MEMORY_BUFFER];
} Memory;

typedef struct {
    u32  len;
    char buffer[];
} String;

typedef struct {
    u32 len;
    u32 buffer[];
} Array;

#define PRINT_STRING(string) printf("%.*s\n", (i32)string->len, string->buffer)

#define SET_START_END(memory, start, type_parent, type_data)           \
    {                                                                  \
        start = memory->len;                                           \
        usize gap = start % alignof(type_parent);                      \
        if (gap != 0) {                                                \
            start += alignof(type_parent) - gap;                       \
        }                                                              \
        end = start + sizeof(type_parent) + (len * sizeof(type_data)); \
        if (CAP_MEMORY_BUFFER <= end) {                                \
            return NULL;                                               \
        }                                                              \
        memory->len = end;                                             \
    }

static String* alloc_copy_string(Memory* memory, u32 len, const char* string) {
    usize start;
    usize end;
    SET_START_END(memory, start, String, char);
    String* x = (String*)&(memory->buffer[start]);
    x->len = len;
    memcpy(&x->buffer, string, len);
    return x;
}

#define ALLOC_EMPTY(fn, type_parent, type_data)                  \
    static type_parent* fn(Memory* memory, u32 len) {            \
        usize start;                                             \
        usize end;                                               \
        SET_START_END(memory, start, type_parent, type_data);    \
        type_parent* x = (type_parent*)&(memory->buffer[start]); \
        x->len = len;                                            \
        return x;                                                \
    }

ALLOC_EMPTY(alloc_empty_string, String, char)
ALLOC_EMPTY(alloc_empty_array, Array, u32)

#define PRINT_ARRAY(fmt, array, len)      \
    {                                     \
        printf("[ ");                     \
        for (usize i = 0; i < len; ++i) { \
            printf(fmt " ", array[i]);    \
        }                                 \
        printf("]\n");                    \
    }

i32 main(void) {
    Memory* memory = calloc(1, sizeof(Memory));
    String* x = alloc_copy_string(memory,
                                  sizeof("Hello, world!") - 1,
                                  "Hello, world!");
    Array*  y = alloc_empty_array(memory, 5);
    if (!y) {
        return EXIT_FAILURE;
    }
    {
        u32 len = (u32)y->len;
        for (u32 i = 0; i < len; ++i) {
            y->buffer[i] = len - i;
        }
    }
    String* z = alloc_empty_string(memory, sizeof("Goodbye!") - 1);
    {
        if (!z) {
            return EXIT_FAILURE;
        }
        memcpy(&z->buffer, "Goodbye!", sizeof("Goodbye!") - 1);
    }
    PRINT_ARRAY("%hhu", memory->buffer, CAP_MEMORY_BUFFER);
    PRINT_STRING(x);
    PRINT_ARRAY("%u", y->buffer, y->len);
    PRINT_STRING(z);
    printf("memory->len : %zu\n", memory->len);
    free(memory);
    return EXIT_SUCCESS;
}
