#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAP_NODES 16

typedef uint8_t u8;
typedef int32_t i32;

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef struct {
    u8 key;
    u8 priority;
} Node;

typedef struct {
    Node nodes[CAP_NODES];
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

static void show(Heap* heap) {
    printf("[ ");
    for (u8 i = 0; i < heap->len_nodes; ++i) {
        printf("%hhu ", heap->nodes[i].priority);
    }
    printf("]\n");
}

#define SWAP(nodes, i, j)    \
    {                        \
        Node t = nodes[i];   \
        nodes[i] = nodes[j]; \
        nodes[j] = t;        \
    }

static void balance_up(Heap* heap, u8 i) {
    u8 j = (u8)(((i + 1) / 2) - 1);
    while (0 < i) {
        if (heap->nodes[i].priority < heap->nodes[j].priority) {
            SWAP(heap->nodes, i, j);
        }
        i = j;
        j = (u8)(((i + 1) / 2) - 1);
    }
}

static void balance_down(Heap* heap, u8 i) {
    for (;;) {
        u8 l = (u8)(((i + 1) * 2) - 1);
        u8 r = l + 1;
        u8 m = i;
        if ((l < heap->len_nodes) &&
            (heap->nodes[l].priority < heap->nodes[m].priority))
        {
            m = l;
        }
        if ((r < heap->len_nodes) &&
            (heap->nodes[r].priority < heap->nodes[m].priority))
        {
            m = r;
        }
        if (i == m) {
            return;
        }
        SWAP(heap->nodes, i, m);
        i = m;
    }
}

static void insert(Heap* heap, Node node) {
    EXIT_IF(CAP_NODES <= heap->len_nodes);
    u8 n = heap->len_nodes;
    heap->nodes[heap->len_nodes++] = node;
    balance_up(heap, n);
}

static Node pop(Heap* heap) {
    EXIT_IF(heap->len_nodes == 0);
    Node node = heap->nodes[0];
    heap->nodes[0] = heap->nodes[--heap->len_nodes];
    balance_down(heap, 0);
    return node;
}

static void drop(Heap* heap, u8 key) {
    EXIT_IF(heap->len_nodes == 0);
    u8 i = 0;
    for (; i < heap->len_nodes; ++i) {
        if (heap->nodes[i].key == key) {
            break;
        }
    }
    EXIT_IF(i == heap->len_nodes);
    u8 previous = heap->nodes[i].priority;
    heap->nodes[i] = heap->nodes[--heap->len_nodes];
    if (previous < heap->nodes[i].priority) {
        balance_down(heap, i);
    } else {
        balance_up(heap, i);
    }
}

Bool check(Heap*, u8);
Bool check(Heap* heap, u8 i) {
    if (heap->len_nodes <= i) {
        return TRUE;
    }
    u8 l = (u8)(((i + 1) * 2) - 1);
    u8 r = l + 1;
    if ((l < heap->len_nodes) &&
        (heap->nodes[l].priority < heap->nodes[i].priority))
    {
        return FALSE;
    }
    if ((r < heap->len_nodes) &&
        (heap->nodes[r].priority < heap->nodes[i].priority))
    {
        return FALSE;
    }
    return check(heap, l) && check(heap, r);
}

#define INSERT(heap, x)                                \
    {                                                  \
        insert(heap, (Node){.key = x, .priority = x}); \
        EXIT_IF(!check(heap, 0));                      \
    }

#define DROP(heap, key)           \
    {                             \
        drop(heap, key);          \
        EXIT_IF(!check(heap, 0)); \
    }

i32 main(void) {
    Heap* heap = calloc(1, sizeof(Heap));
    EXIT_IF(!heap);
    {
        INSERT(heap, 8);
        INSERT(heap, 7);
        DROP(heap, 7);
        DROP(heap, 8);
        INSERT(heap, 7);
        INSERT(heap, 8);
        INSERT(heap, 1);
        INSERT(heap, 4);
        INSERT(heap, 1);
        DROP(heap, 1);
        INSERT(heap, 3);
        INSERT(heap, 12);
        INSERT(heap, 20);
        INSERT(heap, 4);
        INSERT(heap, 20);
        DROP(heap, 4);
        INSERT(heap, 19);
        INSERT(heap, 3);
        DROP(heap, 3);
        INSERT(heap, 0);
        DROP(heap, 20);
        DROP(heap, 20);
        INSERT(heap, 20);
        INSERT(heap, 0);
        INSERT(heap, 25);
        DROP(heap, 0);
    }
    {
        u8 previous = 0;
        while (heap->len_nodes != 0) {
            show(heap);
            u8 current = pop(heap).priority;
            printf("%hhu\n", current);
            EXIT_IF((!check(heap, 0)) || (current < previous));
            previous = current;
        }
        printf("\n");
    }
    free(heap);
    return EXIT_SUCCESS;
}
