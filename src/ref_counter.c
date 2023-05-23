#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t i32;

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

#define OK    0
#define ERROR 1

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

#define EXIT_IF(condition)             \
    do {                               \
        if (condition) {               \
            fprintf(stderr,            \
                    "%s:%s:%d `%s`\n", \
                    __FILE__,          \
                    __func__,          \
                    __LINE__,          \
                    #condition);       \
            _exit(ERROR);              \
        }                              \
    } while (0)

STATIC_ASSERT(sizeof(u64) == sizeof(void*));

typedef struct {
    u64* array;
    u32  len;
    u32  cap;
    u8   bitmap;
    u8   parents;
    Bool alive;
} Block;

#define CAP_HEAP (1 << 8)

static u64 HEAP[CAP_HEAP];
static u32 LEN_HEAP = 0;

#define CAP_BLOCKS (1 << 8)
#define CAP_STACK  CAP_BLOCKS
#define CAP_ROOTS  CAP_BLOCKS

static Block BLOCKS[CAP_BLOCKS];
static u32   LEN_BLOCKS = 0;

static Block* STACK[CAP_STACK];
static u32    LEN_STACK = 0;

static Block* ROOTS[CAP_ROOTS];
static u32    LEN_ROOTS = 0;

static u32 block_index(Block* block) {
    return (u32)(block - &BLOCKS[0]);
}

static u64* alloc_array(u32 len) {
    EXIT_IF(len == 0);
    EXIT_IF(CAP_HEAP <= (LEN_HEAP + len));
    u64* array = &HEAP[LEN_HEAP];
    LEN_HEAP += len;
    return array;
}

static Block* alloc_block(u32 len) {
    EXIT_IF(len == 0);
    EXIT_IF(CAP_BLOCKS <= LEN_BLOCKS);
    Block* block = NULL;
    for (u32 i = 0; i < LEN_BLOCKS; ++i) {
        block = &BLOCKS[i];
        if ((len <= block->cap) && ((block->parents == 0) || (!block->alive)))
        {
            EXIT_IF(block->cap < block->len);
            break;
        }
        block = NULL;
    }
    if (!block) {
        block = &BLOCKS[LEN_BLOCKS++];
    }
    block->array = alloc_array(len);
    block->len = len;
    block->cap = len;
    block->bitmap = 0;
    block->parents = 0;
    block->alive = TRUE;
    return block;
}

static void push_root(Block* block) {
    EXIT_IF(CAP_ROOTS <= LEN_ROOTS);
    ROOTS[LEN_ROOTS++] = block;
}

static Block* alloc_root(u32 len) {
    Block* block = alloc_block(len);
    ++block->parents;
    push_root(block);
    return block;
}

static void free_block(Block* block) {
    if ((block->parents == 0) || (!block->alive)) {
        return;
    }
    --block->parents;
    if (block->parents != 0) {
        return;
    }
    for (u32 i = 0; i < 32; ++i) {
        const u32 bit = 1u << i;
        if ((block->bitmap & bit) == 0) {
            continue;
        }
        free_block((Block*)block->array[i]);
    }
}

static void mark(Block* block) {
    if (block->alive) {
        return;
    }
    block->alive = TRUE;
    for (u32 i = 0; i < 32; ++i) {
        const u32 bit = 1u << i;
        if ((block->bitmap & bit) == 0) {
            continue;
        }
        mark((Block*)block->array[i]);
    }
}

static void pop_root(void) {
    EXIT_IF(LEN_ROOTS == 0);
    free_block(ROOTS[--LEN_ROOTS]);
    for (u32 i = 0; i < LEN_BLOCKS; ++i) {
        BLOCKS[i].alive = FALSE;
    }
    for (u32 i = 0; i < LEN_ROOTS; ++i) {
        mark(ROOTS[i]);
    }
    for (u32 i = 0; i < LEN_BLOCKS; ++i) {
        if (!BLOCKS[i].alive) {
            BLOCKS[i].parents = 0;
        }
    }
}

static void remove_child_at(Block* parent, u32 index) {
    EXIT_IF((sizeof(((Block*)(0))->bitmap) * 8) <= index);
    EXIT_IF(parent->len <= index);
    const u32 bit = 1u << index;
    if ((parent->bitmap & bit) == 0) {
        return;
    }
    parent->bitmap &= ~bit;
    free_block((Block*)parent->array[index]);
}

static void insert_child_at(Block* parent, Block* child, u32 index) {
    EXIT_IF(!parent->alive);
    EXIT_IF(!child->alive);
    EXIT_IF(parent == child);
    EXIT_IF((sizeof(((Block*)(0))->bitmap) * 8) <= index);
    EXIT_IF(parent->len <= index);
    remove_child_at(parent, index);
    parent->array[index] = *(u64*)&child;
    const u32 bit = 1u << index;
    parent->bitmap |= bit;
    ++child->parents;
}

static void _print_trace(Block* block, u32 indent) {
    EXIT_IF(!block->alive);
    EXIT_IF(CAP_STACK <= LEN_STACK);
    for (u32 i = 0; i < LEN_STACK; ++i) {
        if (block == STACK[i]) {
            return;
        }
    }
    STACK[LEN_STACK++] = block;
    for (u32 _ = 0; _ < indent; ++_) {
        putchar(' ');
    }
    printf("- #%u (parents: %hhu)\n", block_index(block), block->parents);
    for (u32 i = 0; i < 32; ++i) {
        const u32 bit = 1u << i;
        if ((block->bitmap & bit) != 0) {
            _print_trace((Block*)block->array[i], indent + 2);
        }
    }
}

static void print_trace(Block* block) {
    LEN_STACK = 0;
    _print_trace(block, 0);
}

static void print_blocks(void) {
    putchar('\n');
    for (u32 i = 0; i < LEN_BLOCKS; ++i) {
        Block* block = &BLOCKS[i];
        printf("[ #%u (parents: %hhu, %s) ]\n",
               block_index(block),
               block->parents,
               block->alive ? "alive" : "dead");
    }
}

i32 main(void) {
    {
        Block* x0 = alloc_root(7);
        (void)x0;
        {
            Block* x1 = alloc_root(2);
            {
                Block* x2 = alloc_root(2);
                {
                    Block* x3 = alloc_root(3);

                    insert_child_at(x2, x3, 1);
                    insert_child_at(x3, x1, 2);
                    insert_child_at(x1, x2, 0);

                    insert_child_at(x1, alloc_block(1), 1);
                    insert_child_at(x2, alloc_block(1), 0);

                    pop_root();
                }
                print_trace(x0);
                print_trace(x1);
                print_blocks();

                pop_root();
            }
            print_blocks();

            pop_root();
        }
        print_blocks();

        pop_root();
    }
    return OK;
}
