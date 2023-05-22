#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef int32_t i32;

typedef uint32_t u32;
typedef uint64_t u64;

typedef struct timespec Time;

#define OK    0
#define ERROR 1

#define NANO_PER_SECOND  1000000000llu
#define MICRO_PER_SECOND 1000000llu
#define NANO_PER_MICRO   (NANO_PER_SECOND / MICRO_PER_SECOND)

#define EXIT_IF(condition)         \
    if (condition) {               \
        fflush(stdout);            \
        fprintf(stderr,            \
                "%s:%s:%d `%s`\n", \
                __FILE__,          \
                __func__,          \
                __LINE__,          \
                #condition);       \
        _exit(ERROR);              \
    }

static u64 get_monotonic(void) {
    Time time;
    EXIT_IF(clock_gettime(CLOCK_MONOTONIC, &time));
    return (((u64)time.tv_sec) * NANO_PER_SECOND) + ((u64)time.tv_nsec);
}

i32 main(void) {
    u64 seconds = 2;
    u64 now = get_monotonic();
    u64 wake_at = get_monotonic() + (seconds * NANO_PER_SECOND);
    EXIT_IF(usleep((u32)((wake_at - now) / NANO_PER_MICRO)));
    return OK;
}
