#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRING_SIZE 16

typedef struct {
    char string[STRING_SIZE];
} a_t;

typedef struct {
    double value;
} b_t;

static void set_a_block(a_t* block, const char* string) {
    if (STRING_SIZE <= strlen(string)) {
        exit(EXIT_FAILURE);
    }
    strcpy(block->string, string);
}

int main(void) {
    void* pool = malloc((sizeof(a_t) * 3) + (sizeof(b_t) * 3));
    if (pool == NULL) {
        return EXIT_FAILURE;
    }
    a_t* a_blocks;
    b_t* b_blocks;
    {
        a_blocks = (a_t*)pool;
        set_a_block(&a_blocks[0], "Hello!");
        set_a_block(&a_blocks[1], "Hello?");
        set_a_block(&a_blocks[2], "...");
    }
    {
        size_t offset =
            (sizeof(a_t) < sizeof(void*)) ? 1 : (sizeof(a_t) / sizeof(void*));
        b_blocks = (b_t*)pool + (3 * offset);
        b_blocks[0].value = 0.1;
        b_blocks[1].value = 0.2;
        b_blocks[2].value = 0.3;
    }
    for (uint8_t i = 0; i < 3; ++i) {
        printf("%s\t%.1f\n", a_blocks[i].string, b_blocks[i].value);
    }
    free(pool);
    return EXIT_SUCCESS;
}
