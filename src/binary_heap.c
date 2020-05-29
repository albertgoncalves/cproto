#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define HEAP_CAP 16

typedef struct {
    u8 key;
} Node;

typedef struct {
    Node nodes[HEAP_CAP];
    u8   length;
} Heap;

static const Node EMPTY = {0};

static void swap(Node* nodes, u8 i, u8 j) {
    Node node = nodes[i];
    nodes[i] = nodes[j];
    nodes[j] = node;
}

static void insert(Heap* heap, Node node) {
    if ((heap->length == HEAP_CAP) || (node.key == 0)) {
        exit(EXIT_FAILURE);
    }
    heap->nodes[heap->length] = node;
    if (0 < heap->length) {
        u8 i = heap->length;
        u8 j = i / 2;
        while (0 < i) {
            if (heap->nodes[j].key < heap->nodes[i].key) {
                swap(heap->nodes, i, j);
            }
            i = j;
            j = i / 2;
        }
    }
    ++heap->length;
    return;
}

static Node pop(Heap* heap) {
    if (heap->length == 0) {
        exit(EXIT_FAILURE);
    }
    Node node = heap->nodes[0];
    u8   length = --heap->length;
    heap->nodes[0] = heap->nodes[length];
    heap->nodes[length] = EMPTY;
    u8 i = 0;
    u8 j = 1;
    while (j < length) {
        if (heap->nodes[j].key < heap->nodes[j + 1].key) {
            ++j;
        }
        if (heap->nodes[j].key <= heap->nodes[i].key) {
            break;
        }
        swap(heap->nodes, i, j);
        i = j;
        j = (u8)(i * 2);
    }
    return node;
}

#define INSERT(heap, key)   \
    {                       \
        Node node = {key};  \
        insert(heap, node); \
    }

int main(void) {
    Heap* heap = calloc(1, sizeof(Heap));
    if (heap == NULL) {
        return EXIT_FAILURE;
    }
    INSERT(heap, 3);
    INSERT(heap, 8);
    INSERT(heap, 21);
    INSERT(heap, 1);
    INSERT(heap, 1);
    INSERT(heap, 4);
    INSERT(heap, 3);
    INSERT(heap, 12);
    INSERT(heap, 20);
    INSERT(heap, 4);
    INSERT(heap, 20);
    INSERT(heap, 19);
    INSERT(heap, 3);
    INSERT(heap, 11);
    u8 length = heap->length;
    for (u8 i = 0; i < length; ++i) {
        printf("pop() : %hhu\n", pop(heap).key);
    }
    free(heap);
    return EXIT_SUCCESS;
}
