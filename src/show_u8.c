#include <stdio.h>
#include <stdlib.h>

static const char CHARS[13] = {
    115,
    114,
    99,
    47,
    109,
    97,
    105,
    110,
    46,
    101,
    114,
    108,
    0,
};

int main(void) {
    printf("\"%s\"\n", (char*)(&CHARS));
    return EXIT_SUCCESS;
}
