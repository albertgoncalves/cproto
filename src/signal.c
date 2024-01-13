#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef int32_t i32;

typedef struct timespec   Time;
typedef timer_t           Timer;
typedef struct itimerspec Interval;

typedef siginfo_t        SigInfo;
typedef struct sigaction SigAction;
typedef struct sigevent  SigEvent;

#define OK 0

static void callback(i32, SigInfo*, void*) {
    printf("   - Callback fired!\n");
}

i32 main(void) {
    SigAction action = {
        .sa_sigaction = callback,
        .sa_flags = SA_SIGINFO,
    };
    SigEvent event = {
        .sigev_notify = SIGEV_SIGNAL,
        .sigev_signo = SIGRTMIN,
        .sigev_value.sival_int = 0,
    };
    Timer timer = {0};

    sigemptyset(&action.sa_mask);
    sigaction(SIGRTMIN, &action, NULL);
    timer_create(CLOCK_MONOTONIC, &event, &timer);

    const Interval interval = {
        {0, 0},
        {0, 500000000},
    };
    printf(" > Setting timer\n");
    timer_settime(timer, 0, &interval, NULL);

    printf(" > Sleeping\n");

    Time end = {0};
    clock_gettime(CLOCK_MONOTONIC, &end);
    ++end.tv_sec;

    i32 result;
    do {
        result = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &end, NULL);
    } while (result != 0);

    printf(" > Done!\n");
    return OK;
}
