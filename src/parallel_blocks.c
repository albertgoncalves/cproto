#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef uint8_t              u8;
typedef uint16_t             u16;
typedef atomic_uint_fast16_t u16Atomic;
typedef pthread_t            Thread;

#define BUFFER_CAP 512u
#define BLOCKS_CAP 512u

#define BUFFER_WIDTH  17u
#define BUFFER_HEIGHT 17u
#define BUFFER_LEN    (BUFFER_WIDTH * BUFFER_HEIGHT)

#define N            4u
#define BLOCK_WIDTH  (BUFFER_WIDTH / N)
#define BLOCK_HEIGHT (BUFFER_HEIGHT / N)
#define X_BLOCKS     ((BUFFER_WIDTH + BLOCK_WIDTH - 1u) / BLOCK_WIDTH)
#define Y_BLOCKS     ((BUFFER_HEIGHT + BLOCK_HEIGHT - 1u) / BLOCK_HEIGHT)
#define BLOCKS_LEN   (X_BLOCKS * Y_BLOCKS)

#define THREAD_CAP 3u

static u16Atomic INDEX = 0;

typedef struct {
    u16 x;
    u16 y;
} XY;

typedef struct {
    XY start;
    XY end;
} Block;

typedef struct {
    u16*   buffer;
    Block* blocks;
    u8     id;
} Payload;

typedef struct {
    Payload payloads[THREAD_CAP];
    Thread  threads[THREAD_CAP];
    Block   blocks[BLOCKS_CAP];
    u16     buffer[BUFFER_CAP];
} Memory;

static void set_buffer(u16* buffer, Block block, u16 value) {
    for (u16 y = block.start.y; y < block.end.y; ++y) {
        u16 offset = (u16)(y * BUFFER_WIDTH);
        for (u16 x = block.start.x; x < block.end.x; ++x) {
            buffer[x + offset] = value;
        }
    }
    usleep(100000);
}

static void* do_work(void* args) {
    Payload* payload = args;
    u16*     buffer = payload->buffer;
    for (;;) {
        u16 index = (u16)atomic_fetch_add(&INDEX, 1);
        if (BLOCKS_LEN <= index) {
            return NULL;
        }
        set_buffer(buffer, payload->blocks[index], index);
    }
}

int main(void) {
    printf("\n"
           "sizeof(u16)       : %zu\n"
           "sizeof(u16Atomic) : %zu\n"
           "sizeof(Thread)    : %zu\n"
           "sizeof(XY)        : %zu\n"
           "sizeof(Block)     : %zu\n"
           "sizeof(Payload)   : %zu\n"
           "sizeof(Memory)    : %zu\n"
           "\n",
           sizeof(u16),
           sizeof(u16Atomic),
           sizeof(Thread),
           sizeof(XY),
           sizeof(Block),
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
    payload.blocks = memory->blocks;
    payload.id = 0;
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
                Block block = {
                    .start = start,
                    .end = end,
                };
                payload.blocks[index++] = block;
            }
        }
    }
    for (u8 i = 0; i < THREAD_CAP; ++i) {
        memory->payloads[i] = payload;
        memory->payloads[i].id = i;
        pthread_create(&memory->threads[i],
                       NULL,
                       do_work,
                       &memory->payloads[i]);
    }
    for (u8 i = 0; i < THREAD_CAP; ++i) {
        pthread_join(memory->threads[i], NULL);
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
