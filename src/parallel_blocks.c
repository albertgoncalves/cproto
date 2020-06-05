#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef atomic_ushort  u16Atomic;
typedef pthread_t      Thread;

#define BUFFER_CAP 512
#define BLOCKS_CAP 512

#define BUFFER_WIDTH  17
#define BUFFER_HEIGHT 17
static const u16 BUFFER_LEN = BUFFER_WIDTH * BUFFER_HEIGHT;

#define N 4
static const u16 BLOCK_WIDTH = BUFFER_WIDTH / N;
static const u16 BLOCK_HEIGHT = BUFFER_HEIGHT / N;
static const u16 X_BLOCKS = (BUFFER_WIDTH + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
static const u16 Y_BLOCKS = (BUFFER_HEIGHT + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;
static const u16 BLOCKS_LEN = X_BLOCKS * Y_BLOCKS;

#define THREAD_CAP 3

static u16Atomic INDEX = 0;

typedef struct {
    u16 x;
    u16 y;
} XY;

typedef struct {
    XY start;
    XY end;
} Work;

typedef struct {
    u16*  buffer;
    Work* works;
    u8    id;
} Payload;

typedef struct {
    u16     buffer[BUFFER_CAP];
    Work    works[BLOCKS_CAP];
    Payload payloads[THREAD_CAP];
    Thread  threads[THREAD_CAP];
} Memory;

static void* set_buffer(void* args) {
    Payload* payload = (Payload*)args;
    for (;;) {
        u16 index = (u16)atomic_fetch_add(&INDEX, 1);
        if (BLOCKS_LEN <= index) {
            return NULL;
        }
        Work work = payload->works[index];
        for (u16 y = work.start.y; y < work.end.y; ++y) {
            u16 offset = (u16)(y * BUFFER_WIDTH);
            for (u16 x = work.start.x; x < work.end.x; ++x) {
                payload->buffer[x + offset] = index;
            }
        }
        usleep(100000);
    }
}

int main(void) {
    printf("\n"
           "sizeof(u16)       : %zu\n"
           "sizeof(u16Atomic) : %zu\n"
           "sizeof(Thread)    : %zu\n"
           "sizeof(XY)        : %zu\n"
           "sizeof(Work)      : %zu\n"
           "sizeof(Payload)   : %zu\n"
           "sizeof(Memory)    : %zu\n"
           "\n",
           sizeof(u16),
           sizeof(u16Atomic),
           sizeof(Thread),
           sizeof(XY),
           sizeof(Work),
           sizeof(Payload),
           sizeof(Memory));
    if ((BUFFER_CAP < BUFFER_LEN) || (BLOCKS_CAP < BLOCKS_LEN)) {
        return EXIT_FAILURE;
    }
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    Payload payload;
    payload.buffer = memory->buffer;
    payload.works = memory->works;
    {
        u16 index = 0;
        for (u16 y = 0; y < Y_BLOCKS; ++y) {
            for (u16 x = 0; x < X_BLOCKS; ++x) {
                XY start = {
                    .x = (u16)(x * BLOCK_WIDTH),
                    .y = (u16)(y * BLOCK_HEIGHT),
                };
                XY end = {
                    .x = (u16)(start.x + BLOCK_WIDTH),
                    .y = (u16)(start.y + BLOCK_HEIGHT),
                };
                end.x = end.x < BUFFER_WIDTH ? end.x : BUFFER_WIDTH;
                end.y = end.y < BUFFER_HEIGHT ? end.y : BUFFER_HEIGHT;
                Work work = {
                    .start = start,
                    .end = end,
                };
                payload.works[index++] = work;
            }
        }
    }
    Thread* threads = memory->threads;
    for (u8 i = 0; i < THREAD_CAP; ++i) {
        memory->payloads[i] = payload;
        memory->payloads[i].id = i;
        pthread_create(&threads[i], NULL, set_buffer, &memory->payloads[i]);
    }
    for (u8 i = 0; i < THREAD_CAP; ++i) {
        pthread_join(threads[i], NULL);
    }
    for (u16 y = 0; y < BUFFER_HEIGHT; ++y) {
        u16 offset = (u16)(y * BUFFER_WIDTH);
        for (u16 x = 0; x < BUFFER_WIDTH; ++x) {
            printf("%3d", payload.buffer[x + offset]);
        }
        printf("\n");
    }
    free(memory);
    return EXIT_SUCCESS;
}
