#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef int32_t i32;

#define OK    0
#define ERROR 1

#define BUFFER_CAP 10

#define EXIT_WITH_TRACE()                                            \
    do {                                                             \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        void*  buffer[BUFFER_CAP];                                   \
        i32    n = backtrace(buffer, BUFFER_CAP);                    \
        char** symbols = backtrace_symbols(buffer, n);               \
        if (symbols) {                                               \
            for (i32 i = 0; i < n; ++i) {                            \
                fprintf(stderr, "\t%s\n", symbols[i]);               \
            }                                                        \
        }                                                            \
        _exit(ERROR);                                                \
    } while (0)

i32 main(void) {
    EXIT_WITH_TRACE();
    return OK;
}
