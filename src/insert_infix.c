#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* NOTE: Need this for
 * `https://deniskyashif.com/2019/02/17/implementing-a-regular-expression-engine/`.
 */

typedef unsigned char u8;

#define INFIX_OPERATOR '.'

static char* get_infix(const char* expression) {
    u8 n = (u8)strlen(expression);
    if (n == 0) {
        exit(EXIT_FAILURE);
    }
    char* buffer = calloc(n * 2u, sizeof(char));
    u8    m = (u8)(n - 1u);
    u8    buffer_index = 0;
    for (u8 i = 0; i < n; ++i) {
        char token = expression[i];
        buffer[buffer_index++] = token;
        if ((token == '(') || (token == '|')) {
            continue;
        }
        if (i < m) {
            char peek = expression[i + 1];
            if ((peek == '*') || (peek == '|') || (peek == ')')) {
                continue;
            }
            buffer[buffer_index++] = INFIX_OPERATOR;
        }
    }
    return buffer;
}

int main(void) {
    const char* input = "((a|b)cd)efg";
    char*       output = get_infix(input);
    printf("%s\n%s\n", input, output);
    free(output);
    return EXIT_SUCCESS;
}
