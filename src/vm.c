#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;
typedef int           i32;

#define U8_MAX (u8)0xFF

typedef enum {
    HALT = 1,
    PUSH,
    POP,
    ADD,
} Instruction;

#define STACK_CAP 64

typedef struct {
    i32 stack[STACK_CAP];
    i32 result;
    u8  stack_len;
} Memory;

#define PROGRAM_LEN 10

static const i32 PROGRAM[PROGRAM_LEN] = {
    PUSH,
    5,
    PUSH,
    6,
    ADD,
    PUSH,
    -1,
    ADD,
    POP,
    HALT,
};

static i32* run(Memory* memory) {
    i32* stack = memory->stack;
    i32* result = NULL;
    for (u8 i = 0; i < PROGRAM_LEN; ++i) {
        i32 instruction = PROGRAM[i];
        switch (instruction) {
        case HALT: {
            printf("%2hhu - HALT\n", i);
            return result;
        }
        case PUSH: {
            if (STACK_CAP == memory->stack_len) {
                exit(EXIT_FAILURE);
            }
            u8  j = i;
            i32 x = PROGRAM[++i];
            printf("%2hhu - PUSH  %2d\n", j, x);
            stack[memory->stack_len++] = x;
            break;
        }
        case POP: {
            i32 x = stack[--memory->stack_len];
            memory->result = x;
            result = &memory->result;
            printf("%2hhu - POP   %2d\n", i, x);
            break;
        }
        case ADD: {
            if (memory->stack_len < 2) {
                exit(EXIT_FAILURE);
            }
            i32 x = stack[--memory->stack_len];
            i32 y = stack[--memory->stack_len];
            printf("%2hhu - ADD   %2d, %2d\n", i, x, y);
            stack[memory->stack_len++] = x + y;
            break;
        }
        default: {
            printf("%2hhu - ?\n", i);
            exit(EXIT_FAILURE);
        }
        }
    }
    return result;
}

int main(void) {
    printf("\n"
           "U8_MAX              : %hhu\n"
           "sizeof(Instruction) : %zu\n"
           "sizeof(Memory)      : %zu\n"
           "\n",
           U8_MAX,
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
