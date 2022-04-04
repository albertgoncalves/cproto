#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint32_t u32;
typedef int32_t  i32;

#define STACK_CAP 64

typedef struct {
    i32 either;
    i32 concat;
} OpCounter;

#define OP_CONCAT       '.'
#define OP_EITHER       '|'
#define OP_ZERO_OR_MANY '*'
#define OP_ZERO_OR_ONE  '?'
#define OP_ONE_OR_MANY  '+'

static char* get_postfix(const char* expression) {
    i32 n = (i32)strlen(expression);
    if (n == 0) {
        exit(EXIT_FAILURE);
    }
    OpCounter counter = {0};
    OpCounter stack[STACK_CAP] = {0};
    i32       stack_index = 0;
    char*     buffer = calloc(((u32)n) * 2u, sizeof(char));
    if (buffer == NULL) {
        exit(EXIT_FAILURE);
    }
    i32 buffer_index = 0;
    for (i32 i = 0; i < n; ++i) {
        char token = expression[i];
        switch (token) {
        case OP_CONCAT: {
            exit(EXIT_FAILURE);
        }
        case '(': {
            if (STACK_CAP <= stack_index) {
                exit(EXIT_FAILURE);
            }
            if (1 < counter.concat) {
                --counter.concat;
                buffer[buffer_index++] = OP_CONCAT;
            }
            stack[stack_index++] = counter;
            counter.concat = 0;
            counter.either = 0;
            break;
        }
        case OP_EITHER: {
            if (counter.concat == 0) {
                exit(EXIT_FAILURE);
            }
            while (0 < --counter.concat) {
                buffer[buffer_index++] = OP_CONCAT;
            }
            ++counter.either;
            break;
        }
        case ')': {
            if ((stack_index == 0) || (counter.concat == 0)) {
                exit(EXIT_FAILURE);
            }
            while (0 < --counter.concat) {
                buffer[buffer_index++] = OP_CONCAT;
            }
            while (0 < counter.either--) {
                buffer[buffer_index++] = OP_EITHER;
            }
            counter = stack[--stack_index];
            ++counter.concat;
            break;
        }
        case OP_ZERO_OR_MANY: {
            if (counter.concat == 0) {
                exit(EXIT_FAILURE);
            }
            buffer[buffer_index++] = OP_ZERO_OR_MANY;
            break;
        }
        case OP_ONE_OR_MANY: {
            if (counter.concat == 0) {
                exit(EXIT_FAILURE);
            }
            buffer[buffer_index++] = OP_ONE_OR_MANY;
            break;
        }
        case OP_ZERO_OR_ONE: {
            if (counter.concat == 0) {
                exit(EXIT_FAILURE);
            }
            buffer[buffer_index++] = OP_ZERO_OR_ONE;
            break;
        }
        default: {
            if (1 < counter.concat) {
                --counter.concat;
                buffer[buffer_index++] = OP_CONCAT;
            }
            buffer[buffer_index++] = token;
            ++counter.concat;
        }
        }
    }
    if (stack_index != 0) {
        exit(EXIT_FAILURE);
    }
    while (0 < --counter.concat) {
        buffer[buffer_index++] = OP_CONCAT;
    }
    while (0 < counter.either--) {
        buffer[buffer_index++] = OP_EITHER;
    }
    return buffer;
}

i32 main(void) {
    const char* input = "a(b|c)*d";
    char*       output = get_postfix(input);
    printf("%s -> %s\n", input, output);
    free(output);
    return EXIT_SUCCESS;
}
