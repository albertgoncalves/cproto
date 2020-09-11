#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int32_t i32;

#define U8_MAX  (u8)0xFF
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

#define PRINT_ERROR_AND_EXIT            \
    printf("\033[1;31mError\033[0m\n"); \
    exit(EXIT_FAILURE)
#define PRINT_INSTR(string) \
    printf("    %2hhu - \033[1m%-6s\033[0m", i, string);
#define PRINT_P1 printf("%4d\n", p1)
#define POP_2                            \
    if (memory->stack_len < 2) {         \
        PRINT_ERROR_AND_EXIT;            \
    }                                    \
    i32 s1 = stack[--memory->stack_len]; \
    i32 s2 = stack[--memory->stack_len]

static void run_program(Memory* memory) {
    i32* stack = memory->stack;
    for (u8 i = 0; i < PROG_LEN;) {
        InstrI32 instruction = PROGRAM[i];
        switch (instruction) {
        case HALT: {
            PRINT_INSTR("HALT");
            if (memory->stack_len != 1) {
                PRINT_ERROR_AND_EXIT;
            }
            printf("\n");
            return;
        }
        case PUSH: {
            PRINT_INSTR("PUSH");
            if ((STACK_CAP <= memory->stack_len) || ((PROG_LEN - 1) <= i)) {
                PRINT_ERROR_AND_EXIT;
            }
            i32 p1 = PROGRAM[i + 1];
            PRINT_P1;
            stack[memory->stack_len++] = p1;
            i = (u8)(i + 2);
            break;
        }
        case POP: {
            PRINT_INSTR("POP");
            if (memory->stack_len < 1) {
                PRINT_ERROR_AND_EXIT;
            }
            printf("\n");
            --memory->stack_len;
            ++i;
            break;
        }
        case COPY: {
            PRINT_INSTR("COPY");
            if ((memory->stack_len < 1) || (STACK_CAP <= memory->stack_len)) {
                PRINT_ERROR_AND_EXIT;
            }
            printf("\n");
            i32 s1 = stack[memory->stack_len - 1];
            stack[memory->stack_len++] = s1;
            ++i;
            break;
        }
        case JUMP0: {
            PRINT_INSTR("JUMP0");
            if ((memory->stack_len < 1) || ((PROG_LEN - 1) <= i)) {
                PRINT_ERROR_AND_EXIT;
            }
            InstrI32 p1 = PROGRAM[i + 1];
            PRINT_P1;
            if (stack[--memory->stack_len] == 0) {
                i = p1;
            } else {
                i = (u8)(i + 2);
            }
            break;
        }
        case LT: {
            PRINT_INSTR("LT");
            POP_2;
            printf("\n");
            stack[memory->stack_len++] = s1 < s2 ? 1 : 0;
            ++i;
            break;
        }
        case ADD: {
            PRINT_INSTR("ADD");
            POP_2;
            printf("\n");
            stack[memory->stack_len++] = s1 + s2;
            ++i;
            break;
        }
        default: {
            PRINT_INSTR("?");
            PRINT_ERROR_AND_EXIT;
        }
        }
    }
    return;
}

#undef PRINT_ERROR_AND_EXIT
#undef PRINT_INSTR
#undef PRINT_P1
#undef POP_2

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
    printf("\nmemory->\033[1mstack\033[0m[\033[1m0\033[0m] : "
           "\033[1;34m%d\033[0m\n",
           memory->stack[0]);
    free(memory);
    return EXIT_SUCCESS;
}
