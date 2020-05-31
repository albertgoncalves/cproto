#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define U8_MAX (u8)0xFF

typedef int i32;

#define I32_MIN (i32)0x80000000

typedef enum {
    HALT = I32_MIN,
    PUSH,
    POP,
    DUPE,
    JUMP0,
    JUMP1,
    EQ,
    LT,
    ADD,
} Instruction;

#define STACK_CAP 64

typedef struct {
    i32 data[STACK_CAP];
    i32 result;
    u8  data_len;
} Memory;

#define PROGRAM_LEN 16

static const Instruction PROGRAM[PROGRAM_LEN] = {
    PUSH,
    5,
    PUSH,
    6,
    ADD,
    DUPE,
    PUSH,
    20,
    LT,
    JUMP0,
    1,
    PUSH,
    -1,
    ADD,
    POP,
    HALT,
};

#define POP_X                   \
    if (memory->data_len < 1) { \
        exit(EXIT_FAILURE);     \
    }                           \
    i32 x = data[--memory->data_len]

#define POP_XY                        \
    if (memory->data_len < 2) {       \
        exit(EXIT_FAILURE);           \
    }                                 \
    i32 x = data[--memory->data_len]; \
    i32 y = data[--memory->data_len]

#define JUMP_IF(y)                           \
    if ((PROGRAM_LEN - 1) <= i) {            \
        exit(EXIT_FAILURE);                  \
    }                                        \
    POP_X;                                   \
    Instruction j = PROGRAM[i + 1];          \
    printf("%3hhu - JUMP%d %3d\n", i, y, j); \
    if (x == y) {                            \
        i = j;                               \
    } else {                                 \
        ++i;                                 \
    }

static i32* run(Memory* memory) {
    i32* data = memory->data;
    i32* result = NULL;
    for (u8 i = 0; i < PROGRAM_LEN; ++i) {
        Instruction instruction = PROGRAM[i];
        switch (instruction) {
        case HALT: {
            printf("%3hhu - HALT\n", i);
            return result;
        }
        case PUSH: {
            if ((STACK_CAP <= memory->data_len) || ((PROGRAM_LEN - 1) <= i)) {
                exit(EXIT_FAILURE);
            }
            u8  j = i;
            i32 x = PROGRAM[++i];
            printf("%3hhu - PUSH  %3d\n", j, x);
            data[memory->data_len++] = x;
            break;
        }
        case POP: {
            POP_X;
            printf("%3hhu - POP   %3d\n", i, x);
            memory->result = x;
            result = &memory->result;
            break;
        }
        case DUPE: {
            if (STACK_CAP <= memory->data_len) {
                exit(EXIT_FAILURE);
            }
            i32 x = data[memory->data_len - 1];
            printf("%3hhu - DUPE  %3d\n", i, x);
            data[memory->data_len++] = x;
            break;
        }
        case JUMP0: {
            JUMP_IF(0);
            break;
        }
        case JUMP1: {
            JUMP_IF(1);
            break;
        }
        case EQ: {
            POP_XY;
            printf("%3hhu - EQ    %3d, %2d\n", i, x, y);
            data[memory->data_len++] = x == y ? 1 : 0;
            break;
        }
        case LT: {
            POP_XY;
            printf("%3hhu - LT    %3d, %2d\n", i, x, y);
            data[memory->data_len++] = x < y ? 1 : 0;
            break;
        }
        case ADD: {
            POP_XY;
            printf("%3hhu - ADD   %3d, %2d\n", i, x, y);
            data[memory->data_len++] = x + y;
            break;
        }
        default: {
            printf("%3hhu - ?\n", i);
            exit(EXIT_FAILURE);
        }
        }
    }
    return result;
}

#undef POP_X
#undef POP_XY
#undef IF

int main(void) {
    printf("\n"
           "U8_MAX              : %hhu\n"
           "I32_MIN             : %d\n"
           "sizeof(Instruction) : %zu\n"
           "sizeof(Memory)      : %zu\n"
           "\n",
           U8_MAX,
           I32_MIN,
           sizeof(Instruction),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    i32* result = run(memory);
    if (result != NULL) {
        printf("\nresult : %d\n", *result);
    }
    free(memory);
    return EXIT_SUCCESS;
}
