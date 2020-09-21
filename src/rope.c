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

typedef union {
    Pair pair;
    char value;
} Data;

struct Rope {
    Data data;
    Type type;
    u8   weight;
};

#define ROPE_CAP 128u
#define N_STACKS 2u

typedef struct {
    Rope  ropes[ROPE_CAP];
    u8    len;
    Rope* stacks[N_STACKS][ROPE_CAP];
} Memory;

static Rope* leaf(Memory* memory, char value) {
    if (memory->len == ROPE_CAP) {
        exit(EXIT_FAILURE);
    }
    Rope* rope = &memory->ropes[memory->len++];
    rope->weight = 1;
    rope->type = LEAF;
    rope->data.value = value;
    return rope;
}

static Rope* concat(Memory* memory, Rope* left, Rope* right) {
    if (memory->len == ROPE_CAP) {
        exit(EXIT_FAILURE);
    }
    if (left == NULL) {
        return right;
    }
    if (right == NULL) {
        return left;
    }
    Rope* rope = &memory->ropes[memory->len++];
    rope->weight = (u8)(left->weight + right->weight);
    rope->type = NODE;
    rope->data.pair.left = left;
    rope->data.pair.right = right;
    return rope;
}

Rope* init(Memory*, const char*, u8, u8);
Rope* init(Memory* memory, const char* string, u8 i, u8 j) {
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
    } else {
        u8 m = (u8)(((j - i) / 2) + i);
        return concat(memory,
                      init(memory, string, i, m),
                      init(memory, string, m, j));
    }
    return NULL;
}

Pair split(Memory*, Rope*, i8);
Pair split(Memory* memory, Rope* rope, i8 i) {
    Pair pair = {0};
    if (rope == NULL) {
        return pair;
    }
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
        i8 weight = (i8)rope->data.pair.left->weight;
        if (i < weight) {
            pair = split(memory, rope->data.pair.left, i);
            pair.right = concat(memory, pair.right, rope->data.pair.right);
        } else {
            pair = split(memory, rope->data.pair.right, (i8)(i - weight));
            pair.left = concat(memory, rope->data.pair.left, pair.left);
        }
        break;
    }
    }
    return pair;
}

static void print_string(Memory* memory, Rope* rope) {
    Rope** left_stack = memory->stacks[0];
    Rope** right_stack = memory->stacks[1];
    u8     i = 0;
    u8     j = 0;
    left_stack[i++] = rope;
    printf("\"");
    for (;;) {
        if (i != 0) {
            rope = left_stack[--i];
        } else if (j != 0) {
            rope = right_stack[--j];
        } else {
            break;
        }
        if (rope == NULL) {
            break;
        }
        switch (rope->type) {
        case LEAF: {
            printf("%c", rope->data.value);
            break;
        }
        case NODE: {
            left_stack[i++] = rope->data.pair.left;
            right_stack[j++] = rope->data.pair.right;
            break;
        }
        }
    }
    printf("\"\n");
}

void _print_rope(Rope*);
void _print_rope(Rope* rope) {
    if (rope == NULL) {
        return;
    }
    switch (rope->type) {
    case LEAF: {
        printf("Leaf(\'%c\')", rope->data.value);
        break;
    }
    case NODE: {
        printf("Node(%hhu, ", rope->weight);
        _print_rope(rope->data.pair.left);
        printf(", ");
        _print_rope(rope->data.pair.right);
        printf(")");
        break;
    }
    }
}

static void print_rope(Rope* rope) {
    _print_rope(rope);
    printf("\n");
}

static u8 len(const char* string) {
    u8 i = 0;
    while (string[i] != '\0') {
        ++i;
    }
    return i;
}

void ping(Rope*, i8);
void ping(Rope* rope, i8 i) {
    if (rope == NULL) {
        return;
    }
    switch (rope->type) {
    case LEAF: {
        printf("\'%c\'\n", rope->data.value);
        break;
    }
    case NODE: {
        i8 weight = (i8)rope->data.pair.left->weight;
        if (i < weight) {
            ping(rope->data.pair.left, i);
        } else {
            ping(rope->data.pair.right, (i8)(i - weight));
        }
        break;
    }
    }
}

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
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    const char* string = "abcdefghijklmnopqrstuvwxyz";
    Rope*       rope = init(memory, string, 0, len(string));
    printf("\n");
    print_rope(rope);
    printf("\n");
    ping(rope, 5);
    ping(rope, 13);
    ping(rope, 20);
    if (rope == NULL) {
        return EXIT_FAILURE;
    }
    Pair pair = split(memory, rope, 26);
    if (pair.left != NULL) {
        printf("\n%hhu\n", pair.left->weight);
        print_string(memory, pair.left);
    }
    if (pair.right != NULL) {
        printf("\n%hhu\n", pair.right->weight);
        print_string(memory, pair.right);
    }
    printf("%hhu\n", memory->len);
    free(memory);
    return EXIT_SUCCESS;
}
