#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef uint8_t  u8;
typedef uint64_t u64;

typedef int32_t i32;
typedef int64_t i64;

#define OK    0
#define ERROR 1

#define EXIT_IF(condition)            \
    do {                              \
        if (condition) {              \
            printf("%s:%s:%d `%s`\n", \
                   __FILE__,          \
                   __func__,          \
                   __LINE__,          \
                   #condition);       \
            _exit(ERROR);             \
        }                             \
    } while (0)

i32 main(void) {
    /*
        void f(i64* x, i64* y) {
            *x += *y;
        }
    */
    const u8 bytes[] = {0x48, 0x8B, 0x06, 0x48, 0x01, 0x07, 0xC3};
    // NOTE: Alignment is not necessary, but doesn't hurt to know this trick.
    const u64 size = (sizeof(bytes) + (8lu - 1lu)) & (~(8lu - 1lu));

    void* func = mmap(NULL,
                      size,
                      PROT_READ | PROT_WRITE,
                      MAP_ANONYMOUS | MAP_PRIVATE,
                      -1,
                      0);
    EXIT_IF(func == MAP_FAILED);
    memcpy(func, bytes, sizeof(bytes));
    EXIT_IF(mprotect(func, size, PROT_EXEC));

    i64 a = 1017;
    i64 b = -118;

    printf("%-6ld %-6ld\n", a, b);

    ((void (*)(i64*, i64*))func)(&a, &b);
    ((void (*)(i64*, i64*))func)(&b, &a);

    printf("%-6ld %-6ld\n", a, b);

    return OK;
}
