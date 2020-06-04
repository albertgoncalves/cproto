#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef pthread_mutex_t Mutex;
typedef pthread_t       Thread;

#define BUFFER_WIDTH  20
#define BUFFER_HEIGHT 20
#define BUFFER_CAP    (BUFFER_WIDTH * BUFFER_HEIGHT)

#define N            3
#define BLOCK_WIDTH  (BUFFER_WIDTH / N)
#define BLOCK_HEIGHT (BUFFER_HEIGHT / N)
#define X_BLOCKS     ((BUFFER_WIDTH + BLOCK_WIDTH - 1) / BLOCK_WIDTH)
#define Y_BLOCKS     ((BUFFER_HEIGHT + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT)
#define BLOCKS_CAP   (X_BLOCKS * Y_BLOCKS)

#define N_THREADS 3

static u16   INDEX = 0;
static Mutex LOCK;

static u16 sync_index(void) {
    pthread_mutex_lock(&LOCK);
    u16 index = INDEX++;
    pthread_mutex_unlock(&LOCK);
    return index;
}

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
} Payload;

static void* set_buffer(void* args) {
    Payload* payload = (Payload*)args;
    for (;;) {
        u16 index = sync_index();
        if (BLOCKS_CAP <= index) {
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

#define MIN(a, b) a < b ? a : b

int main(void) {
    u16* buffer = calloc(BUFFER_CAP, sizeof(u16));
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }
    Work* works = calloc(BLOCKS_CAP, sizeof(Work));
    if (works == NULL) {
        return EXIT_FAILURE;
    }
    {
        u16 index = 0;
        for (u16 y = 0; y < Y_BLOCKS; ++y) {
            for (u16 x = 0; x < X_BLOCKS; ++x) {
                XY start = {
                    .x = (u16)(x * BLOCK_WIDTH),
                    .y = (u16)(y * BLOCK_HEIGHT),
                };
                XY end = {
                    .x = MIN((u16)(start.x + BLOCK_WIDTH), BUFFER_WIDTH),
                    .y = MIN((u16)(start.y + BLOCK_HEIGHT), BUFFER_HEIGHT),
                };
                Work work = {
                    .start = start,
                    .end = end,
                };
                works[index++] = work;
            }
        }
    }
    Payload payload = {
        .buffer = buffer,
        .works = works,
    };
    Thread threads[N_THREADS];
    for (u8 i = 0; i < N_THREADS; ++i) {
        pthread_create(&threads[i], NULL, set_buffer, &payload);
    }
    for (u8 i = 0; i < N_THREADS; ++i) {
        pthread_join(threads[i], NULL);
    }
    for (u16 y = 0; y < BUFFER_HEIGHT; ++y) {
        u16 offset = (u16)(y * BUFFER_WIDTH);
        for (u16 x = 0; x < BUFFER_WIDTH; ++x) {
            printf("%3d", buffer[x + offset]);
        }
        printf("\n");
    }
    free(buffer);
    free(works);
    return EXIT_SUCCESS;
}
