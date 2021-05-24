#include <stdalign.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int32_t i32;

#define CAP_MEMORY_BUFFER (2 << 5)

typedef struct {
    u32 len;
    u8  buffer[CAP_MEMORY_BUFFER];
} Memory;

typedef struct {
    u32  len;
    char buffer[];
} String;

typedef struct {
    u32 len;
    u32 buffer[];
} Array;

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "\n%s:%s:%d `%s`\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
    }

#define PRINT_STRING(string) printf("%.*s\n", (i32)string->len, string->buffer)

#define SET_START_END(memory, start, type_parent, type_data)           \
    {                                                                  \
        start = memory->len;                                           \
        u32 gap = start % alignof(type_parent);                        \
        if (gap != 0) {                                                \
            start += alignof(type_parent) - gap;                       \
        }                                                              \
        end = start + sizeof(type_parent) + (len * sizeof(type_data)); \
        EXIT_IF(CAP_MEMORY_BUFFER <= end);                             \
        memory->len = end;                                             \
    }

static String* alloc_copy_string(Memory* memory, u32 len, const char* string) {
    u32 start;
    u32 end;
    SET_START_END(memory, start, String, char);
    String* x = (String*)&(memory->buffer[start]);
    x->len = len;
    memcpy(&x->buffer, string, len);
    return x;
}

#define ALLOC_EMPTY(fn, type_parent, type_data)                  \
    static type_parent* fn(Memory* memory, u32 len) {            \
        u32 start;                                               \
        u32 end;                                                 \
        SET_START_END(memory, start, type_parent, type_data);    \
        type_parent* x = (type_parent*)&(memory->buffer[start]); \
        x->len = len;                                            \
        return x;                                                \
    }

ALLOC_EMPTY(alloc_empty_string, String, char)
ALLOC_EMPTY(alloc_empty_array, Array, u32)

#define PRINT_ARRAY(fmt, array, len)    \
    {                                   \
        printf("[ ");                   \
        for (u32 i = 0; i < len; ++i) { \
            printf(fmt " ", array[i]);  \
        }                               \
        printf("]\n");                  \
    }

i32 main(void) {
    Memory* memory = calloc(1, sizeof(Memory));
    EXIT_IF(!memory);
    Array*  x;
    String* y;
    String* z;
    {
        y = alloc_copy_string(memory,
                              sizeof("Hello, world!") - 1,
                              "Hello, world!");
    }
    {
        x = alloc_empty_array(memory, 5);
        for (u32 i = 0; i < x->len; ++i) {
            x->buffer[i] = x->len - i;
        }
    }
    {
        u32 n = sizeof("Goodbye!") - 1;
        z = alloc_empty_string(memory, n);
        memcpy(&z->buffer, "Goodbye!", n);
    }
    PRINT_ARRAY("%hhu", memory->buffer, CAP_MEMORY_BUFFER);
    PRINT_ARRAY("%u", x->buffer, x->len);
    PRINT_STRING(y);
    PRINT_STRING(z);
    printf("memory->len : %u\n", memory->len);
    free(memory);
    return EXIT_SUCCESS;
}
