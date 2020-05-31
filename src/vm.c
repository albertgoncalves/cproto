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
    COPY,
    JUMP0,
    LT,
    ADD,
} InstrI32;

#define STACK_CAP 4

typedef struct {
    i32 stack[STACK_CAP];
    u8  stack_len;
} Memory;

#define PROG_LEN 18

static const InstrI32 PROGRAM[PROG_LEN] = {
    PUSH,
    5,
    PUSH,
    6,
    ADD,
    COPY,
    PUSH,
    20,
    LT,
    JUMP0,
    2,
    PUSH,
    -1,
    ADD,
    PUSH,
    21,
    POP,
    HALT,
};

#define INC1 ++i
#define INC2 i = (u8)(i + 2)

#define POP_2                            \
    if (memory->stack_len < 2) {         \
        exit(EXIT_FAILURE);              \
    }                                    \
    i32 s1 = stack[--memory->stack_len]; \
    i32 s2 = stack[--memory->stack_len]

#define JUMP_IF(x)                            \
    if ((PROG_LEN - 1) <= i) {                \
        exit(EXIT_FAILURE);                   \
    }                                         \
    if (memory->stack_len < 1) {              \
        exit(EXIT_FAILURE);                   \
    }                                         \
    i32      s1 = stack[--memory->stack_len]; \
    InstrI32 p1 = PROGRAM[i + 1];             \
    printf("%3hhu - JUMP%d %3d\n", i, x, p1); \
    if (s1 == x) {                            \
        i = p1;                               \
    } else {                                  \
        INC2;                                 \
    }

static void run_program(Memory* memory) {
    i32* stack = memory->stack;
    for (u8 i = 0; i < PROG_LEN;) {
        InstrI32 instruction = PROGRAM[i];
        switch (instruction) {
        case HALT: {
            if (memory->stack_len != 1) {
                exit(EXIT_FAILURE);
            }
            printf("%3hhu - HALT\n", i);
            return;
        }
        case PUSH: {
            if ((STACK_CAP <= memory->stack_len) || ((PROG_LEN - 1) <= i)) {
                exit(EXIT_FAILURE);
            }
            i32 p1 = PROGRAM[i + 1];
            printf("%3hhu - PUSH  %3d\n", i, p1);
            stack[memory->stack_len++] = p1;
            INC2;
            break;
        }
        case POP: {
            printf("%3hhu - POP\n", i);
            if (memory->stack_len < 1) {
                exit(EXIT_FAILURE);
            }
            --memory->stack_len;
            INC1;
            break;
        }
        case COPY: {
            printf("%3hhu - COPY\n", i);
            if ((memory->stack_len < 1) || (STACK_CAP <= memory->stack_len)) {
                exit(EXIT_FAILURE);
            }
            i32 s1 = stack[memory->stack_len - 1];
            stack[memory->stack_len++] = s1;
            INC1;
            break;
        }
        case JUMP0: {
            JUMP_IF(0);
            break;
        }
        case LT: {
            printf("%3hhu - LT\n", i);
            POP_2;
            stack[memory->stack_len++] = s1 < s2 ? 1 : 0;
            INC1;
            break;
        }
        case ADD: {
            printf("%3hhu - ADD\n", i);
            POP_2;
            stack[memory->stack_len++] = s1 + s2;
            INC1;
            break;
        }
        default: {
            printf("%3hhu - ?\n", i);
            exit(EXIT_FAILURE);
        }
        }
    }
    return;
}

#undef INC1
#undef INC2
#undef POP_2
#undef JUMP_IF

int main(void) {
    printf("\n"
           "U8_MAX           : %hhu\n"
           "I32_MIN          : %d\n"
           "sizeof(i32)      : %zu\n"
           "sizeof(InstrI32) : %zu\n"
           "sizeof(Memory)   : %zu\n"
           "\n",
           U8_MAX,
           I32_MIN,
           sizeof(i32),
           sizeof(InstrI32),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    run_program(memory);
    printf("\nmemory->stack[0] : %d\n", memory->stack[0]);
    free(memory);
    return EXIT_SUCCESS;
}
