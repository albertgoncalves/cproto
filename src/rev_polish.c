#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef float         f32;

static f32 eval_rev_polish(const char* expression, f32* stack, char* buffer) {
    u8 stack_index = 0;
    u8 buffer_index = 0;
    for (char token = *expression++; token != '\0'; token = *expression++) {
        switch (token) {
        case '+': {
            if (stack_index < 2) {
                exit(EXIT_FAILURE);
            }
            f32 b = stack[--stack_index];
            f32 a = stack[--stack_index];
            stack[stack_index++] = a + b;
            break;
        }
        case '-': {
            if (stack_index < 2) {
                exit(EXIT_FAILURE);
            }
            f32 b = stack[--stack_index];
            f32 a = stack[--stack_index];
            stack[stack_index++] = a - b;
            break;
        }
        case '*': {
            if (stack_index < 2) {
                exit(EXIT_FAILURE);
            }
            f32 b = stack[--stack_index];
            f32 a = stack[--stack_index];
            stack[stack_index++] = a * b;
            break;
        }
        case '/': {
            if (stack_index < 2) {
                exit(EXIT_FAILURE);
            }
            f32 b = stack[--stack_index];
            f32 a = stack[--stack_index];
            stack[stack_index++] = a / b;
            break;
        }
        case '^': {
            if (stack_index < 2) {
                exit(EXIT_FAILURE);
            }
            f32 b = stack[--stack_index];
            f32 a = stack[--stack_index];
            stack[stack_index++] = powf(a, b);
            break;
        }
        case ' ': {
            if (buffer_index == 0) {
                continue;
            }
            stack[stack_index++] = strtof(buffer, NULL);
            buffer[0] = '\0';
            buffer_index = 0;
            break;
        }
        case '.': {
            buffer[buffer_index++] = '.';
            buffer[buffer_index] = '\0';
            break;
        }
        default: {
            if (('0' <= token) && (token <= '9')) {
                buffer[buffer_index++] = token;
                buffer[buffer_index] = '\0';
            } else {
                exit(EXIT_FAILURE);
            }
        }
        }
    }
    if ((stack_index != 1) || (buffer_index != 0)) {
        exit(EXIT_FAILURE);
    }
    return stack[0];
}

int main(void) {
    /* NOTE: Shunting-yard of `3 + 4 * 2 / (1 - 5) ^ 2 ^ 3`. */
    const char* expression = "3 4 2 * 1 5 - 2 3 ^ ^ / +";
    {
        u8    n = (u8)strlen(expression);
        void* pool = calloc(n, sizeof(f32) + sizeof(char));
        f32*  stack = (f32*)pool;
        char* buffer = (char*)&stack[n];
        printf("%.8f\n", (double)eval_rev_polish(expression, stack, buffer));
        free(pool);
    }
    return EXIT_SUCCESS;
}
