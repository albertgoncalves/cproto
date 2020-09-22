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
};

static Rope* leaf(char value) {
    Rope* rope = malloc(sizeof(Rope));
    if (rope == NULL) {
        exit(EXIT_FAILURE);
    }
    rope->weight = 1;
    rope->type = LEAF;
    rope->items.value = value;
    return rope;
}

static Rope* concat(Rope* left, Rope* right) {
    if (left == NULL) {
        return right;
    }
    if (right == NULL) {
        return left;
    }
    Rope* rope = malloc(sizeof(Rope));
    if (rope == NULL) {
        exit(EXIT_FAILURE);
    }
    rope->weight = (u8)(left->weight + right->weight);
    rope->type = NODE;
    rope->items.pair.left = left;
    rope->items.pair.right = right;
    return rope;
}

Rope* new_(const char*, u8, u8);
Rope* new_(const char* string, u8 i, u8 j) {
    if (i == j) {
        return NULL;
    }
    if (j < i) {
        u8 x = i;
        i = j;
        j = x;
    }
    if ((j - i) == 1) {
        return leaf(string[i]);
    }
    u8 m = (u8)(((j - i) / 2) + i);
    return concat(new_(string, i, m), new_(string, m, j));
}

Pair _split(Rope*, i8);
Pair _split(Rope* rope, i8 i) {
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
        free(rope);
        if (i < weight) {
            pair = _split(left, i);
            pair.right = concat(pair.right, right);
        } else {
            pair = _split(right, (i8)(i - weight));
            pair.left = concat(left, pair.left);
        }
        break;
    }
    }
    return pair;
}

static Rope* insert(Rope* a, Rope* b, i8 i) {
    Pair pair = _split(a, i);
    return concat(concat(pair.left, b), pair.right);
}

void free_(Rope*);
void free_(Rope* rope) {
    switch (rope->type) {
    case LEAF: {
        free(rope);
        break;
    }
    case NODE: {
        free_(rope->items.pair.left);
        free_(rope->items.pair.right);
        free(rope);
        break;
    }
    }
}

static Rope* delete_(Rope* rope, i8 i, i8 j) {
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
    Pair  pair = _split(rope, i);
    Rope* left = pair.left;
    Rope* right = pair.right;
    if (right != NULL) {
        pair = _split(right, j);
        free_(pair.left);
        return concat(left, pair.right);
    }
    return left;
}

void ping(Rope*, i8);
void ping(Rope* rope, i8 i) {
    switch (rope->type) {
    case LEAF: {
        printf("\'%c\'\n", rope->items.value);
        break;
    }
    case NODE: {
        i8 weight = (i8)rope->items.pair.left->weight;
        if (i < weight) {
            ping(rope->items.pair.left, i);
        } else {
            ping(rope->items.pair.right, (i8)(i - weight));
        }
        break;
    }
    }
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

int main(void) {
    printf("sizeof(Type)   : %zu\n"
           "sizeof(Rope*)  : %zu\n"
           "sizeof(Pair)   : %zu\n"
           "sizeof(Rope)   : %zu\n",
           sizeof(Type),
           sizeof(Rope*),
           sizeof(Pair),
           sizeof(Rope));
    const char* a = "12345";
    const char* b = "_a_b_c_";
    const char* c = "??";
    Rope*       rope = insert(new_(a, 0, len(a)), new_(b, 0, len(b)), 4);
    rope = delete_(rope, 3, 6);
    rope = insert(rope, new_(c, 0, len(c)), 2);
    rope = delete_(rope, 3, 4);
    printf("\n");
    print_rope(rope);
    printf("\n");
    print_string(rope);
    printf("\n");
    ping(rope, 5);
    printf("\n");
    free_(rope);
    return EXIT_SUCCESS;
}
