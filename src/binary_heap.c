#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef int8_t  i8;
typedef int32_t i32;

#define HEAP_CAP 16

typedef struct {
    i8 key;
} Node;

typedef struct {
    Node nodes[HEAP_CAP];
    u8   len_nodes;
} Heap;

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "\n%s:%s:%d `%s`\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
    }

#define SWAP(nodes, i, j)    \
    {                        \
        Node t = nodes[i];   \
        nodes[i] = nodes[j]; \
        nodes[j] = t;        \
    }

static void insert(Heap* heap, Node node) {
    EXIT_IF(heap->len_nodes == HEAP_CAP);
    heap->nodes[heap->len_nodes++] = node;
    if (heap->len_nodes == 1) {
        return;
    }
    u8 i = heap->len_nodes - 1;
    u8 j = i / 2;
    while (0 < i) {
        if (heap->nodes[i].key < heap->nodes[j].key) {
            SWAP(heap->nodes, i, j);
        }
        i = j;
        j = i / 2;
    }
}

static Node pop(Heap* heap) {
    EXIT_IF(heap->len_nodes == 0);
    Node node = heap->nodes[0];
    u8   n = --heap->len_nodes;
    heap->nodes[0] = heap->nodes[n];
    u8 i = 0;
    u8 j = 1;
    while (j < n) {
        if ((j < (n - 1)) && (heap->nodes[j + 1].key < heap->nodes[j].key)) {
            ++j;
        }
        if (heap->nodes[i].key <= heap->nodes[j].key) {
            break;
        }
        SWAP(heap->nodes, i, j);
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

i32 main(void) {
    Heap* heap = calloc(1, sizeof(Heap));
    EXIT_IF(!heap);
    {
        INSERT(heap, 3);
        INSERT(heap, 8);
        INSERT(heap, 21);
        INSERT(heap, 1);
        INSERT(heap, 4);
        INSERT(heap, 1);
        INSERT(heap, 3);
        INSERT(heap, 12);
        INSERT(heap, 20);
        INSERT(heap, 4);
        INSERT(heap, 20);
        INSERT(heap, 19);
        INSERT(heap, 3);
        INSERT(heap, 0);
        INSERT(heap, 11);
    }
    {
        printf("[");
        u8 n = heap->len_nodes;
        for (u8 i = 0; i < n; ++i) {
            printf(" %hhd", pop(heap).key);
        }
        printf(" ]\n");
    }
    free(heap);
    return EXIT_SUCCESS;
}
