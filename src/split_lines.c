#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef size_t        usize;
typedef long int      i64;

#define BUFFER_CAP 128

typedef struct {
    u8 start;
    u8 end;
} Line;

typedef struct {
    char buffer[BUFFER_CAP];
    Line lines[BUFFER_CAP];
} Memory;

int main(int n, char** args) {
    printf("\n"
           "sizeof(u8)     : %zu\n"
           "sizeof(i64)    : %zu\n"
           "sizeof(Line)   : %zu\n"
           "sizeof(Memory) : %zu\n"
           "\n",
           sizeof(u8),
           sizeof(i64),
           sizeof(Line),
           sizeof(Memory));
    if (n == 1) {
        return EXIT_FAILURE;
    }
    FILE* file = fopen(args[1], "r");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    fseek(file, 0, SEEK_END);
    usize size = (usize)ftell(file);
    fseek(file, 0, SEEK_SET);
    if (BUFFER_CAP < size) {
        return EXIT_FAILURE;
    }
    Memory* memory = calloc(1, sizeof(Memory));
    if (fread(memory->buffer, sizeof(char), size, file) != size) {
        return EXIT_FAILURE;
    }
    u8 i = 0;
    u8 start = 0;
    for (u8 end = 0; end < size; ++end) {
        if (memory->buffer[end] == '\n') {
            Line line = {
                .start = start,
                .end = end,
            };
            memory->lines[i] = line;
            start = (u8)(end + 1);
            ++i;
        }
    }
    for (u8 j = 0; j < i; ++j) {
        Line line = memory->lines[j];
        for (u8 k = line.end; line.start < k; --k) {
            printf("%c", memory->buffer[k - 1]);
        }
        printf("\n");
    }
    fclose(file);
    free(memory);
    return EXIT_SUCCESS;
}
