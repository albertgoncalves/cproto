#include <execinfo.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef int32_t i32;

#define OK    0
#define ERROR 1

#define BUFFER_CAP 16

#define TRACE()                                           \
    do {                                                  \
        void*     buffer[BUFFER_CAP];                     \
        const i32 n = backtrace(buffer, BUFFER_CAP);      \
        char**    symbols = backtrace_symbols(buffer, n); \
        for (i32 i = 0; i < n; ++i) {                     \
            fprintf(stderr, "\t%s\n", symbols[i]);        \
        }                                                 \
    } while (0)

#define EXIT_WITH_TRACE()                                            \
    do {                                                             \
        fflush(stdout);                                              \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        TRACE();                                                     \
        fflush(stderr);                                              \
        _exit(ERROR);                                                \
    } while (0)

__attribute__((noreturn)) void child(void);
__attribute__((noreturn)) void child(void) {
    EXIT_WITH_TRACE();
}

__attribute__((noreturn)) void parent(void);
__attribute__((noreturn)) void parent(void) {
    child();
}

i32 main(void) {
    parent();
    return OK;
}
