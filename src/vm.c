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
    STORE,
    LOAD,
    JUMP0,
    JUMP1,
    EQ,
    LT,
    ADD,
} InstrI32;

#define STACK_CAP  4
#define GLOBAL_CAP 4

typedef struct {
    i32 stack[STACK_CAP];
    i32 global[GLOBAL_CAP];
    i32 result;
    u8  stack_len;
    u8  global_len;
} Memory;

#define PROG_LEN 25

static const InstrI32 PROGRAM[PROG_LEN] = {
    PUSH, 5,  STORE, 0,     LOAD, 0,    PUSH, 6,    ADD, STORE, 0,   LOAD, 0,
    PUSH, 20, LT,    JUMP0, 4,    LOAD, 0,    PUSH, -1,  ADD,   POP, HALT,
};

#define INC1 ++i
#define INC2 i = (u8)(i + 2)

#define POP_1                    \
    if (memory->stack_len < 1) { \
        exit(EXIT_FAILURE);      \
    }                            \
    i32 s1 = stack[--memory->stack_len]

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
    POP_1;                                    \
    InstrI32 p1 = PROGRAM[i + 1];             \
    printf("%3hhu - JUMP%d %3d\n", i, x, p1); \
    if (s1 == x) {                            \
        i = p1;                               \
    } else {                                  \
        INC2;                                 \
    }

static i32* run_program(Memory* memory) {
    i32* stack = memory->stack;
    i32* global = memory->global;
    i32* result = NULL;
    for (u8 i = 0; i < PROG_LEN;) {
        InstrI32 instruction = PROGRAM[i];
        switch (instruction) {
        case HALT: {
            printf("%3hhu - HALT\n", i);
            return result;
        }
        case PUSH: {
            if ((STACK_CAP <= memory->stack_len) || ((PROG_LEN - 1) <= i)) {
                exit(EXIT_FAILURE);
            }
            i32 x = PROGRAM[i + 1];
            printf("%3hhu - PUSH  %3d\n", i, x);
            stack[memory->stack_len++] = x;
            INC2;
            break;
        }
        case POP: {
            printf("%3hhu - POP\n", i);
            POP_1;
            memory->result = s1;
            result = &memory->result;
            INC1;
            break;
        }
        case STORE: {
            if ((GLOBAL_CAP <= memory->global_len) || ((PROG_LEN - 1) <= i)) {
                exit(EXIT_FAILURE);
            }
            POP_1;
            InstrI32 p1 = PROGRAM[i + 1];
            printf("%3hhu - STORE %3d\n", i, p1);
            global[p1] = s1;
            INC2;
            break;
        }
        case LOAD: {
            if ((STACK_CAP <= memory->stack_len) || ((PROG_LEN - 1) <= i)) {
                exit(EXIT_FAILURE);
            }
            InstrI32 x = PROGRAM[i + 1];
            printf("%3hhu - LOAD  %3d\n", i, x);
            if (GLOBAL_CAP <= x) {
                exit(EXIT_FAILURE);
            }
            stack[memory->stack_len++] = global[x];
            INC2;
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
            printf("%3hhu - EQ\n", i);
            POP_2;
            stack[memory->stack_len++] = s1 == s2 ? 1 : 0;
            INC1;
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
    return result;
}

#undef INC1
#undef INC2
#undef POP_1
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
    i32* result = run_program(memory);
    if (result != NULL) {
        printf("\nresult : %d\n", *result);
    }
    free(memory);
    return EXIT_SUCCESS;
}
