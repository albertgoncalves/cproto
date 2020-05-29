#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define HEAP_CAP 16

typedef struct {
    u8 key;
} Node;

typedef struct {
    Node heap[HEAP_CAP];
    u8   length;
} Memory;

static const Node EMPTY = {0};

static void swap(Memory* memory, u8 i, u8 j) {
    Node swap = memory->heap[i];
    memory->heap[i] = memory->heap[j];
    memory->heap[j] = swap;
}

static void insert(Memory* memory, Node node) {
    if ((memory->length == HEAP_CAP) || (node.key == 0)) {
        exit(EXIT_FAILURE);
    }
    memory->heap[memory->length] = node;
    if (0 < memory->length) {
        u8 i = memory->length;
        u8 j = i / 2;
        while (0 < i) {
            if (memory->heap[j].key < memory->heap[i].key) {
                swap(memory, i, j);
            }
            i = j;
            j = i / 2;
        }
    }
    ++memory->length;
    return;
}

static Node pop(Memory* memory) {
    if (memory->length == 0) {
        exit(EXIT_FAILURE);
    }
    Node node = memory->heap[0];
    u8   length = --memory->length;
    memory->heap[0] = memory->heap[length];
    memory->heap[length] = EMPTY;
    u8 i = 0;
    u8 j = 1;
    while (j < length) {
        if (memory->heap[j].key < memory->heap[j + 1].key) {
            ++j;
        }
        if (memory->heap[j].key <= memory->heap[i].key) {
            break;
        }
        swap(memory, i, j);
        i = j;
        j = (u8)(i * 2);
    }
    return node;
}

#define INSERT(memory, key)   \
    {                         \
        Node node = {key};    \
        insert(memory, node); \
    }

int main(void) {
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    INSERT(memory, 3);
    INSERT(memory, 8);
    INSERT(memory, 21);
    INSERT(memory, 1);
    INSERT(memory, 1);
    INSERT(memory, 4);
    INSERT(memory, 3);
    INSERT(memory, 12);
    INSERT(memory, 20);
    INSERT(memory, 4);
    INSERT(memory, 20);
    INSERT(memory, 19);
    INSERT(memory, 3);
    INSERT(memory, 11);
    u8 length = memory->length;
    for (u8 i = 0; i < length; ++i) {
        printf("pop() : %hhu\n", pop(memory).key);
    }
    free(memory);
    return EXIT_SUCCESS;
}
