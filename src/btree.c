#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// NOTE: See `https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html`.

typedef int32_t  i32;
typedef uint32_t u32;

#define OK    0
#define ERROR 1

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

#define STATIC_ASSERT(condition) _Static_assert(condition, "!(" #condition ")")

#define EXIT()                                                       \
    {                                                                \
        fflush(stdout);                                              \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(ERROR);                                                \
    }

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fflush(stdout);              \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        _exit(ERROR);                \
    }

typedef i32         Key;
typedef const char* Value;

#define SHOW_KEY       "%3d"
#define SHOW_VALUE     "\"%s\""
#define SHOW_KEY_VALUE SHOW_KEY ": " SHOW_VALUE

#define CAP_LEAF_BUFFER 5
#define CAP_NODES       4
#define CAP_CHILDREN    (CAP_NODES + 1)

STATIC_ASSERT(1 < CAP_LEAF_BUFFER);
STATIC_ASSERT(2 < CAP_NODES);

#define CAP_BLOCKS (1 << 4)
#define CAP_LEAFS  (1 << 4)

typedef struct {
    Key   key;
    Value value;
} Leaf;

typedef struct Leafs Leafs;

struct Leafs {
    Leaf   buffer[CAP_LEAF_BUFFER];
    u32    len;
    Leafs* next;
};

typedef struct Block Block;

typedef union {
    Leafs* as_leafs;
    Block* as_block;
} Child;

typedef enum {
    CHILD_UNSET = 0,
    CHILD_LEAFS,
    CHILD_BLOCK,
} ChildTag;

struct Block {
    Key      nodes[CAP_NODES];
    u32      len_nodes;
    Child    children[CAP_CHILDREN];
    ChildTag child_tag;
};

typedef struct {
    Block blocks[CAP_BLOCKS];
    u32   len_blocks;
    Leafs leafs[CAP_LEAFS];
    u32   len_leafs;
} Memory;

static Memory* alloc_memory(void) {
    void* memory = mmap(NULL,
                        sizeof(Memory),
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,
                        0);
    EXIT_IF(memory == MAP_FAILED);
    return (Memory*)memory;
}

static Block* alloc_block(Memory* memory) {
    EXIT_IF(CAP_BLOCKS <= memory->len_blocks);
    Block* block = &memory->blocks[memory->len_blocks++];
    block->len_nodes = 0;
    block->child_tag = CHILD_UNSET;
    return block;
}

static Leafs* alloc_leafs(Memory* memory) {
    EXIT_IF(CAP_LEAFS <= memory->len_leafs);
    Leafs* leafs = &memory->leafs[memory->len_leafs++];
    leafs->len = 0;
    leafs->next = NULL;
    return leafs;
}

static Block* new_tree(Memory* memory) {
    Block* tree = alloc_block(memory);
    tree->child_tag = CHILD_LEAFS;
    tree->children[0].as_leafs = alloc_leafs(memory);
    return tree;
}

static void insert_leafs(Leafs* leafs, Key key, Value value) {
    EXIT_IF(CAP_LEAF_BUFFER <= leafs->len);
    if (leafs->len == 0) {
        leafs->buffer[leafs->len++] = (Leaf){key, value};
        return;
    }
    u32 i = 0;
    for (; i < leafs->len; ++i) {
        if (key < leafs->buffer[i].key) {
            break;
        }
        EXIT_IF(key == leafs->buffer[i].key);
    }
    for (u32 j = leafs->len; i < j; --j) {
        leafs->buffer[j] = leafs->buffer[j - 1];
    }
    leafs->buffer[i] = (Leaf){key, value};
    ++leafs->len;
}

static Key move_half_leafs(Leafs* left, Leafs* right) {
    u32 i = CAP_LEAF_BUFFER / 2;
    u32 j = 0;
    for (; i < CAP_LEAF_BUFFER;) {
        right->buffer[j++] = left->buffer[i++];
        --left->len;
        ++right->len;
    }
    left->next = right;
    return right->buffer[0].key;
}

static Key move_half_block(Block* left, Block* right) {
    {
        u32 i = (CAP_NODES / 2) + 1;
        u32 j = 0;
        for (; i < CAP_CHILDREN;) {
            right->children[j++] = left->children[i++];
        }
    }
    {
        u32 i = (CAP_NODES / 2) + 1;
        u32 j = 0;
        --left->len_nodes;
        for (; i < CAP_NODES;) {
            right->nodes[j++] = left->nodes[i++];
            --left->len_nodes;
            ++right->len_nodes;
        }
        return left->nodes[CAP_NODES / 2];
    }
}

// NOTE: The thing that held me up for the longest time was how to push a split
// back *up* through the tree, after having descended down to the lowest level.
// The trick, at least in this implementation, is the insert elements into the
// children while control is at the parent level. If, after inserting a new
// item, the child is now full, that child block is immediately split. This
// structure of inserting and splitting from the parent layer allows us to, in
// effect, propagate a split up through the tree. The draw-back here is wasted
// space (since a given block is always split immediately after becoming full),
// but with the benefit of greatly simplified traversal.
static Block* insert_block(Memory* memory,
                           Block*  block,
                           Key     key,
                           Value   value) {
    EXIT_IF(CAP_NODES <= block->len_nodes);
    if (block->len_nodes == 0) {
        EXIT_IF(block->child_tag != CHILD_LEAFS);
        insert_leafs(block->children[0].as_leafs, key, value);
        if (block->children[0].as_leafs->len == CAP_LEAF_BUFFER) {
            block->children[1].as_leafs = alloc_leafs(memory);
            block->nodes[0] = move_half_leafs(block->children[0].as_leafs,
                                              block->children[1].as_leafs);
            ++block->len_nodes;
        }
        return block;
    } else {
        u32 i = 0;
        for (; i < block->len_nodes; ++i) {
            if (key < block->nodes[i]) {
                break;
            }
        }
        switch (block->child_tag) {
        case CHILD_LEAFS: {
            insert_leafs(block->children[i].as_leafs, key, value);
            if (block->children[i].as_leafs->len == CAP_LEAF_BUFFER) {
                for (u32 j = block->len_nodes; i < j; --j) {
                    block->nodes[j] = block->nodes[j - 1];
                    block->children[j + 1] = block->children[j];
                }
                block->children[i + 1].as_leafs = alloc_leafs(memory);
                Leafs* left_next = block->children[i].as_leafs->next;
                block->nodes[i] =
                    move_half_leafs(block->children[i].as_leafs,
                                    block->children[i + 1].as_leafs);
                block->children[i + 1].as_leafs->next = left_next;
                ++block->len_nodes;
            }
            break;
        }
        case CHILD_BLOCK: {
            block->children[i].as_block =
                insert_block(memory, block->children[i].as_block, key, value);
            Block* left = block->children[i].as_block;
            if (left->len_nodes == CAP_NODES) {
                // NOTE: Only the root node grows vertically; this way the tree
                // never becomes unbalanced. Instead, here we only grow
                // horizontally; since blocks are never allowed to remain full,
                // we always have space to expand into.
                Block* right = alloc_block(memory);
                right->child_tag = left->child_tag;
                Key new_key = move_half_block(left, right);
                u32 j = 0;
                for (; j < block->len_nodes; ++j) {
                    if (new_key < block->nodes[j]) {
                        break;
                    }
                }
                for (u32 k = block->len_nodes; j < k; --k) {
                    block->nodes[k] = block->nodes[k - 1];
                }
                for (u32 k = block->len_nodes + 1; j < k; --k) {
                    block->children[k] = block->children[k - 1];
                }
                ++block->len_nodes;
                block->nodes[j] = new_key;
                block->children[j].as_block = left;
                block->children[j + 1].as_block = right;
            }
            break;
        }
        case CHILD_UNSET:
        default: {
            EXIT();
        }
        }
    }
    return block;
}

static Block* insert_tree(Memory* memory, Block* left, Key key, Value value) {
    left = insert_block(memory, left, key, value);
    if (left->len_nodes == CAP_NODES) {
        // NOTE: If the root node is full, let's grow the tree vertically to
        // make some more room. Since only the root is allowed to grow
        // vertically, the tree will always remain height-balanced.
        Block* right = alloc_block(memory);
        Block* parent = alloc_block(memory);
        parent->child_tag = CHILD_BLOCK;
        right->child_tag = left->child_tag;
        parent->nodes[0] = move_half_block(left, right);
        ++parent->len_nodes;
        parent->children[0].as_block = left;
        parent->children[1].as_block = right;
        return parent;
    }
    return left;
}

static const Value* lookup_leafs(const Leafs* leafs, Key key) {
    for (u32 i = 0; i < leafs->len; ++i) {
        if (key == leafs->buffer[i].key) {
            return &leafs->buffer[i].value;
        }
        if (key < leafs->buffer[i].key) {
            break;
        }
    }
    return NULL;
}

static const Value* lookup_block(const Block* block, Key key) {
    u32 i = 0;
    for (; i < block->len_nodes; ++i) {
        if (key < block->nodes[i]) {
            break;
        }
    }
    switch (block->child_tag) {
    case CHILD_LEAFS: {
        return lookup_leafs(block->children[i].as_leafs, key);
    }
    case CHILD_BLOCK: {
        return lookup_block(block->children[i].as_block, key);
    }
    case CHILD_UNSET:
    default: {
        EXIT();
    }
    }
}

static void print_padding(u32 padding) {
    for (u32 j = 0; j < padding; ++j) {
        putchar(' ');
    }
}

static void print_leafs(const Leafs* leafs, u32 padding) {
    if (!leafs) {
        return;
    }
    for (u32 i = 0; i < leafs->len; ++i) {
        print_padding(padding);
        printf(SHOW_KEY_VALUE "\n",
               leafs->buffer[i].key,
               leafs->buffer[i].value);
    }
}

#define INDENT 4

static void print_block(const Block* block, u32 padding) {
    if (!block) {
        return;
    }
    switch (block->child_tag) {
    case CHILD_LEAFS: {
        for (u32 i = 0;; ++i) {
            print_leafs(block->children[i].as_leafs, padding + INDENT);
            if (block->len_nodes <= i) {
                return;
            }
            print_padding(padding);
            printf(SHOW_KEY "\n", block->nodes[i]);
        }
    }
    case CHILD_BLOCK: {
        for (u32 i = 0;; ++i) {
            print_block(block->children[i].as_block, padding + INDENT);
            if (block->len_nodes <= i) {
                return;
            }
            print_padding(padding);
            printf(SHOW_KEY "\n", block->nodes[i]);
        }
    }
    case CHILD_UNSET:
    default: {
        EXIT();
    }
    }
}

static void print_walk_leafs(const Block* block) {
    switch (block->child_tag) {
    case CHILD_LEAFS: {
        Leafs* leafs = block->children[0].as_leafs;
        for (;;) {
            printf(" { ");
            for (u32 i = 0;;) {
                printf(SHOW_KEY_VALUE,
                       leafs->buffer[i].key,
                       leafs->buffer[i].value);
                ++i;
                if (leafs->len <= i) {
                    break;
                }
                printf(", ");
            }
            printf(" }\n");
            if (!leafs->next) {
                return;
            }
            leafs = leafs->next;
        }
    }
    case CHILD_BLOCK: {
        EXIT_IF(block->len_nodes == 0);
        print_walk_leafs(block->children[0].as_block);
        break;
    }
    case CHILD_UNSET:
    default: {
        EXIT();
    }
    }
}

#undef INDENT

i32 main(void) {
    printf("\n"
           "sizeof(Leafs)  : %zu\n"
           "sizeof(Block)  : %zu\n"
           "sizeof(Memory) : %zu\n",
           sizeof(Leafs),
           sizeof(Block),
           sizeof(Memory));
    {
        Memory* memory = alloc_memory();
        Block*  tree = new_tree(memory);
#define INSERT(key, value)                            \
    {                                                 \
        tree = insert_tree(memory, tree, key, value); \
    }
        INSERT(12, "0");
        INSERT(11, "1");
        INSERT(21, "2");
        INSERT(17, "3");
        INSERT(13, "4");
        INSERT(18, "5");
        INSERT(19, "6");
        INSERT(110, "7");
        INSERT(16, "8");
        INSERT(20, "9");
        INSERT(15, "10");
        INSERT(14, "11");
        INSERT(22, "12");
        INSERT(25, "13");
        INSERT(23, "14");
        INSERT(24, "15");
        INSERT(30, "16");
        INSERT(35, "17");
        INSERT(33, "18");
        INSERT(10, "19");
        INSERT(0, "20");
        INSERT(27, "21");
        INSERT(26, "22");
        INSERT(28, "23");
        INSERT(210, "24");
        INSERT(211, "25");
        INSERT(29, "26");
        INSERT(32, "27");
#undef INSERT
        print_block(tree, 0);
        printf("\n");
        print_walk_leafs(tree);
        printf("\n");
#define LOOKUP(key)                                        \
    {                                                      \
        const Value* value = lookup_block(tree, key);      \
        if (!value) {                                      \
            printf("  " SHOW_KEY ": _\n", key);            \
        } else {                                           \
            printf("  " SHOW_KEY_VALUE "\n", key, *value); \
        }                                                  \
    }
        LOOKUP(31);
        LOOKUP(32);
        LOOKUP(12);
        LOOKUP(22);
        LOOKUP(26);
        LOOKUP(29);
        LOOKUP(210);
        LOOKUP(201);
#undef LOOKUP
    }
    return OK;
}
