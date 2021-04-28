#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t u8;
typedef int32_t i32;

#define X 4
#define Y 4

typedef struct {
    u8 array[X][Y];
} Memory;

i32 main(void) {
    Memory* memory = calloc(1, sizeof(Memory));
    memset(memory->array, 0xFF, sizeof(memory->array));
    for (i32 i = 0; i < X; ++i) {
        for (i32 j = 0; j < Y; ++j) {
            printf("%hhu\n", memory->array[i][j]);
        }
    }
    memset(memory->array, 0, sizeof(memory->array));
    for (i32 i = 0; i < X; ++i) {
        for (i32 j = 0; j < Y; ++j) {
            printf("%hhu\n", memory->array[i][j]);
        }
    }
    free(memory);
    return EXIT_SUCCESS;
}
