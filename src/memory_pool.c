#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 16
#define MEM_SIZE    3

typedef uint8_t u8;
typedef double  f64;

typedef struct {
    char string[STRING_SIZE];
} a_t;

typedef struct {
    f64 value;
} b_t;

typedef struct {
    a_t a[MEM_SIZE];
    b_t b[MEM_SIZE];
} memory_t;

static void set_a(a_t* a, const char* string) {
    if (STRING_SIZE <= strlen(string)) {
        exit(EXIT_FAILURE);
    }
    strcpy(a->string, string);
}

int main(void) {
    memory_t* memory = malloc(sizeof(memory_t));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    a_t* a = memory->a;
    b_t* b = memory->b;
    {
        set_a(&a[0], "Hello!");
        set_a(&a[1], "Hello?");
        set_a(&a[2], "...");
    }
    {
        b[0].value = 0.1;
        b[1].value = 0.2;
        b[2].value = 0.3;
    }
    for (u8 i = 0; i < MEM_SIZE; ++i) {
        printf("%s\t%.1f\n", a[i].string, b[i].value);
    }
    free(memory);
    return EXIT_SUCCESS;
}
