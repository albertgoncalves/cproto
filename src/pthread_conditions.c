#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;

typedef pthread_t       Thread;
typedef pthread_mutex_t Mutex;
typedef pthread_cond_t  Cond;

typedef struct {
    Mutex* mutex;
    Cond*  cond;
} Args;

static void* child(void* args) {
    printf("Child waiting for lock\n");
    assert(pthread_mutex_unlock(((Args*)args)->mutex) == 0);

    printf("Child sending signal\n");
    assert(pthread_cond_signal(((Args*)args)->cond) == 0);

    printf("Child done\n");
    return NULL;
}

i32 main(void) {
    Mutex mutex = PTHREAD_MUTEX_INITIALIZER;
    Cond  cond = PTHREAD_COND_INITIALIZER;

    Args args = {
        .mutex = &mutex,
        .cond = &cond,
    };

    printf("Parent waiting for lock\n");
    assert(pthread_mutex_lock(&mutex) == 0);

    printf("Parent creating child thread\n");
    Thread child_thread = {};
    assert(pthread_create(&child_thread, NULL, child, &args) == 0);
    assert(pthread_detach(child_thread) == 0);

    printf("Parent waiting for signal\n");
    assert(pthread_cond_wait(&cond, &mutex) == 0);

    printf("Parent cleaning up\n");
    assert(pthread_cond_destroy(&cond) == 0);
    assert(pthread_mutex_trylock(&mutex) == EBUSY);
    assert(pthread_mutex_unlock(&mutex) == 0);

    printf("Parent done\n");
    return 0;
}
