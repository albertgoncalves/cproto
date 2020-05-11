#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char u8;
typedef float         f32;

static f32 eval_rev_polish(const char* expression,
                           u8          n,
                           char*       buffer,
                           f32*        stack) {
    u8 stack_index = 0;
    u8 buffer_index = 0;
    for (u8 i = 0; i < n; ++i) {
        char token = expression[i];
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
            buffer_index = 0;
            break;
        }
        default: {
            if ((('0' <= token) && (token <= '9')) || (token == '.')) {
                buffer[buffer_index++] = token;
            }
        }
        }
    }
    f32 result = stack[0];
    return result;
}

int main(void) {
    /* NOTE: Shunting-yard of `3 + 4 * 2 / (1 - 5) ^ 2 ^ 3`. */
    const char* expression = "3 4 2 * 1 5 - 2 3 ^ ^ / +";
    u8          n = (u8)strlen(expression);
    f32*        pool = calloc(sizeof(f32) + sizeof(char), n);
    f32*        stack = pool;
    char*       buffer = (char*)&pool[n];
    f32         result = eval_rev_polish(expression, n, buffer, stack);
    printf("%.8f\n", (double)result);
    free(pool);
    return EXIT_SUCCESS;
}
