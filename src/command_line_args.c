#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;

#define OK 0

i32 main(i32 n, const char** args) {
    for (i32 i = 0; i < n; ++i) {
        printf("%s\n", args[i]);
    }
    return OK;
}
