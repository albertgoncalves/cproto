#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int8_t  i8;

typedef enum {
    LEAF,
    NODE,
} Type;

typedef struct Rope Rope;

typedef struct {
    Rope* left;
    Rope* right;
} Pair;

struct Rope {
    union {
        Pair pair;
        char value;
    } items;
    Type type;
    u8   weight;
    u8   slot;
};

#define ROPE_CAP 64u

typedef struct {
    Rope ropes[ROPE_CAP];
    u8   slots[ROPE_CAP];
    u8   len;
} Memory;

static Memory* init(void) {
    Memory* memory = calloc(1u, sizeof(Memory));
    if (memory == NULL) {
        exit(EXIT_FAILURE);
    }
    for (u8 i = 0; i < ROPE_CAP; ++i) {
        memory->slots[memory->len++] = i;
    }
    return memory;
}

static Rope* alloc(Memory* memory) {
    if (memory->len == 0) {
        exit(EXIT_FAILURE);
    }
    u8    slot = memory->slots[--memory->len];
    Rope* rope = &memory->ropes[slot];
    rope->slot = slot;
    return rope;
}

static void dealloc(Memory* memory, Rope* rope) {
    memory->slots[memory->len++] = rope->slot;
}

static Rope* leaf(Memory* memory, char value) {
    Rope* rope = alloc(memory);
    rope->weight = 1;
    rope->type = LEAF;
    rope->items.value = value;
    return rope;
}

static Rope* concat(Memory* memory, Rope* left, Rope* right) {
    if (left == NULL) {
        return right;
    }
    if (right == NULL) {
        return left;
    }
    Rope* rope = alloc(memory);
    rope->weight = (u8)(left->weight + right->weight);
    rope->type = NODE;
    rope->items.pair.left = left;
    rope->items.pair.right = right;
    return rope;
}

Rope* new_(Memory*, const char*, u8, u8);
Rope* new_(Memory* memory, const char* string, u8 i, u8 j) {
    if (i == j) {
        return NULL;
    }
    if (j < i) {
        u8 x = i;
        i = j;
        j = x;
    }
    if ((j - i) == 1) {
        return leaf(memory, string[i]);
    }
    u8 m = (u8)(((j - i) / 2) + i);
    return concat(memory,
                  new_(memory, string, i, m),
                  new_(memory, string, m, j));
}

Pair _split(Memory*, Rope*, i8);
Pair _split(Memory* memory, Rope* rope, i8 i) {
    Pair pair = {0};
    switch (rope->type) {
    case LEAF: {
        if (i < 1) {
            pair.right = rope;
        } else {
            pair.left = rope;
        }
        break;
    }
    case NODE: {
        i8    weight = (i8)rope->items.pair.left->weight;
        Rope* left = rope->items.pair.left;
        Rope* right = rope->items.pair.right;
        dealloc(memory, rope);
        if (i < weight) {
            pair = _split(memory, left, i);
            pair.right = concat(memory, pair.right, right);
        } else {
            pair = _split(memory, right, (i8)(i - weight));
            pair.left = concat(memory, left, pair.left);
        }
        break;
    }
    }
    return pair;
}

static Rope* insert(Memory* memory, Rope* a, Rope* b, i8 i) {
    Pair pair = _split(memory, a, i);
    return concat(memory, concat(memory, pair.left, b), pair.right);
}

void free_(Memory*, Rope*);
void free_(Memory* memory, Rope* rope) {
    switch (rope->type) {
    case LEAF: {
        dealloc(memory, rope);
        break;
    }
    case NODE: {
        free_(memory, rope->items.pair.left);
        free_(memory, rope->items.pair.right);
        dealloc(memory, rope);
        break;
    }
    }
}

static Rope* delete_(Memory* memory, Rope* rope, i8 i, i8 j) {
    if (i == j) {
        return rope;
    }
    if (j < i) {
        i8 x = i;
        i = j;
        j = (i8)(j - x);
    } else {
        j = (i8)(j - i);
    }
    Pair  pair = _split(memory, rope, i);
    Rope* left = pair.left;
    Rope* right = pair.right;
    if (right != NULL) {
        pair = _split(memory, right, j);
        free_(memory, pair.left);
        return concat(memory, left, pair.right);
    }
    return left;
}

Rope* balance(Memory*, Rope*);
Rope* balance(Memory* memory, Rope* rope) {
    switch (rope->type) {
    case LEAF: {
        break;
    }
    case NODE: {
        Pair  pair = _split(memory, rope, (i8)(rope->weight / 2));
        Rope* left = pair.left;
        Rope* right = pair.right;
        if (left != NULL) {
            left = balance(memory, left);
        }
        if (right != NULL) {
            right = balance(memory, right);
        }
        rope = concat(memory, left, right);
        break;
    }
    }
    return rope;
}

char ping(Rope*, i8);
char ping(Rope* rope, i8 i) {
    char value = '\0';
    switch (rope->type) {
    case LEAF: {
        value = rope->items.value;
        break;
    }
    case NODE: {
        i8 weight = (i8)rope->items.pair.left->weight;
        if (i < weight) {
            value = ping(rope->items.pair.left, i);
        } else {
            value = ping(rope->items.pair.right, (i8)(i - weight));
        }
        break;
    }
    }
    return value;
}

void _print_string(Rope*);
void _print_string(Rope* rope) {
    switch (rope->type) {
    case LEAF: {
        printf("%c", rope->items.value);
        break;
    }
    case NODE: {
        _print_string(rope->items.pair.left);
        _print_string(rope->items.pair.right);
        break;
    }
    }
}

static void indent(u8 n) {
    for (u8 i = 0; i < n; ++i) {
        printf(" ");
    }
}

void _print_rope(Rope*, u8);
void _print_rope(Rope* rope, u8 n) {
    switch (rope->type) {
    case LEAF: {
        indent(n);
        printf("Leaf(\'%c\')", rope->items.value);
        break;
    }
    case NODE: {
        indent(n);
        printf("Node(\n");
        u8 m = (u8)(n + 2);
        indent(m);
        printf("%hhu,\n", rope->weight);
        _print_rope(rope->items.pair.left, m);
        printf(",\n");
        _print_rope(rope->items.pair.right, m);
        printf("\n");
        indent(n);
        printf(")");
        break;
    }
    }
}

static void print_string(Rope* rope) {
    _print_string(rope);
    printf("\n");
}

static void print_rope(Rope* rope) {
    _print_rope(rope, 0);
    printf("\n");
}

static u8 len(const char* string) {
    u8 i = 0;
    while (string[i] != '\0') {
        ++i;
    }
    return i;
}

static const char* STRING = "12345_a_b_c_??";

int main(void) {
    printf("sizeof(Type)   : %zu\n"
           "sizeof(Rope*)  : %zu\n"
           "sizeof(Pair)   : %zu\n"
           "sizeof(Rope)   : %zu\n"
           "sizeof(Memory) : %zu\n",
           sizeof(Type),
           sizeof(Rope*),
           sizeof(Pair),
           sizeof(Rope),
           sizeof(Memory));
    Memory* memory = init();
    Rope*   rope = insert(memory,
                        new_(memory, STRING, 0, 5),
                        new_(memory, STRING, 5, 12),
                        4);
    rope = delete_(memory, rope, 3, 6);
    rope = insert(memory, rope, new_(memory, STRING, 12, 14), 2);
    rope = delete_(memory, rope, 3, 4);
    rope = insert(memory, rope, new_(memory, STRING, 5, 11), 0);
    rope = delete_(memory, rope, -1, 1);
    rope = balance(memory, rope);
    printf("\n");
    print_rope(rope);
    printf("\n");
    print_string(rope);
    printf("\nping(rope, 6) : \'%c\'\n", ping(rope, 6));
    free_(memory, rope);
    printf("\nmemory->len : %hhu\n", memory->len);
    free(memory);
    return EXIT_SUCCESS;
}
