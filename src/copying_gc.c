#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint32_t u32;
typedef uint64_t u64;

STATIC_ASSERT(sizeof(u64) == sizeof(void*));

typedef enum {
    FALSE = 0,
    TRUE = 1,
} Bool;

#define OK    0
#define ERROR 1

#define EXIT_WITH(x)                                                         \
    do {                                                                     \
        fprintf(stderr, "%s:%s:%d `%s`\n", __FILE__, __func__, __LINE__, x); \
        _exit(ERROR);                                                        \
    } while (FALSE)

#define EXIT_IF(condition)         \
    do {                           \
        if (condition) {           \
            EXIT_WITH(#condition); \
        }                          \
    } while (FALSE)

#define CAP_MEMORY (1 << 8)
static void* MEMORY[CAP_MEMORY * 2];

static void** FROM = &MEMORY[0];
static u64    LEN_FROM = 0;

static void** TO = &MEMORY[CAP_MEMORY];

enum {
    DATA_MASK = 7,

    DATA_POINTER_TAG = 0,

    DATA_LITERAL_TAG = 1,
    DATA_LITERAL_SHIFT = 1llu,
};

typedef struct Block Block;

typedef union {
    i64    as_i64;
    u64    as_u64;
    Block* as_block;
    void*  as_raw_pointer;
} Data;

STATIC_ASSERT(sizeof(Data) <= sizeof(void*));

struct Block {
    u64  size : 63;
    Bool forward : 1;
    Data data[];
};

#define CAP_STACK (1 << 5)
static Block** STACK[CAP_STACK];
static u64     LEN_STACK = 0;

static void stack_push(Block** block) {
    EXIT_IF(CAP_STACK <= LEN_STACK);
    STACK[LEN_STACK++] = block;
}

static Block** stack_pop(void) {
    EXIT_IF(LEN_STACK == 0);
    return STACK[--LEN_STACK];
}

static Data pack_literal(Data data) {
    return (Data){
        .as_u64 =
            ((0x7FFFFFFFFFFFFFFFllu & data.as_u64) << DATA_LITERAL_SHIFT) |
            DATA_LITERAL_TAG,
    };
}

static Data unpack_u64(Data data) {
    return (Data){
        .as_u64 = data.as_u64 >> DATA_LITERAL_SHIFT,
    };
}

static Data unpack_i64(Data data) {
    return (Data){
        .as_i64 = data.as_i64 >> DATA_LITERAL_SHIFT,
    };
}

static Bool is_pointer(Data data) {
    EXIT_IF(!data.as_raw_pointer);
    return (data.as_u64 & DATA_MASK) == DATA_POINTER_TAG;
}

STATIC_ASSERT(sizeof(u64) == 8);
STATIC_ASSERT(sizeof(Block) == 8);

#define BLOCK_HEADER_SIZE (sizeof(Block) / sizeof(u64))

static Block* copy(Block* old) {
    EXIT_IF(old->size == 0);

    if (old->forward) {
        return old->data[0].as_block;
    }

    const u64 len = LEN_FROM + BLOCK_HEADER_SIZE + old->size;
    EXIT_IF(CAP_MEMORY < len);

    Block* new = (Block*)&FROM[LEN_FROM];
    new->forward = FALSE;
    new->size = old->size;
    memcpy(&new->data[0], &old->data[0], old->size * sizeof(u64));

    old->forward = TRUE;
    old->data[0].as_block = new;

    LEN_FROM = len;

    return new;
}

static void collect(void) {
    {
        void** swap = FROM;
        FROM = TO;
        TO = swap;
    }
    LEN_FROM = 0;

    for (u64 i = 0; i < LEN_STACK; ++i) {
        Block* old = *STACK[i];
        Block* new = copy(old);
        printf("Copied stack pointer `%p` to `%p`\n", (void*)old, (void*)new);
        *STACK[i] = new;
    }

    for (u64 i = 0; i < LEN_FROM;) {
        Block* root = (Block*)&FROM[i];
        for (u64 j = 0; j < root->size; ++j) {
            if (is_pointer(root->data[j])) {
                Block* old = root->data[j].as_block;

                const Bool forward = old->forward;
                Block* new = copy(old);

                printf("%s child pointer `%p` to `%p`\n",
                       forward ? "Forwarded" : "Copied",
                       (void*)old,
                       (void*)new);
                root->data[j].as_block = new;
            }
        }
        i += BLOCK_HEADER_SIZE + root->size;
    }
}

static Block* alloc(u64 size) {
    EXIT_IF(size == 0);
    EXIT_IF(0x8000000000000000llu <= size);

    u64 len = LEN_FROM + BLOCK_HEADER_SIZE + size;
    if (CAP_MEMORY < len) {
        collect();
        len = LEN_FROM + BLOCK_HEADER_SIZE + size;
    }
    EXIT_IF(CAP_MEMORY < len);

    Block* block = (Block*)&FROM[LEN_FROM];
    block->forward = FALSE;
    block->size = size;

    LEN_FROM = len;

    return block;
}

i32 main(void) {
    Block* parent = alloc(3);
    stack_push(&parent);
    parent->data[0] = pack_literal((Data){.as_u64 = 123});
    parent->data[1] = pack_literal((Data){.as_i64 = -456});
    parent->data[2].as_block = alloc(1);
    parent->data[2].as_block->data[0].as_block = parent;

    collect();
    printf("\n"
           "sizeof(Block)   : %zu\n"
           "\n"
           "LEN_FROM        : %lu\n"
           "parent->forward : %s\n"
           "parent->size    : %lu\n"
           "parent->data[0] : %lu\n"
           "parent->data[1] : %ld\n"
           "parent->data[2] : %p\n"
           "child->data[0]  : %p\n",
           sizeof(Block),
           LEN_FROM,
           parent->forward ? "true" : "false",
           parent->size,
           unpack_u64(parent->data[0]).as_u64,
           unpack_i64(parent->data[1]).as_i64,
           parent->data[2].as_raw_pointer,
           parent->data[2].as_block->data[0].as_raw_pointer);

    stack_pop();
    collect();
    EXIT_IF(LEN_FROM != 0);

    return OK;
}
