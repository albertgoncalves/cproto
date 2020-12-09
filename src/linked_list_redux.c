#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NOTE: See `https://github.com/mkirchner/linked-list-good-taste`. */

typedef uint8_t u8;
typedef int32_t i32;
typedef size_t  usize;

typedef struct Node Node;

struct Node {
    u8    value;
    Node* next;
};

typedef struct {
    Node* head;
} List;

#define COUNT_NODES 10

static Node NODES[COUNT_NODES];

static Node** find(List* list, Node* target) {
    Node** pointer = &list->head;
    while ((*pointer) && ((*pointer) != target)) {
        pointer = &((*pointer)->next);
    }
    return pointer;
}

static void drop(List* list, Node* target) {
    Node** pointer = find(list, target);
    *pointer = target->next;
}

static void insert(List* list, Node* before, Node* node) {
    Node** pointer = find(list, before);
    *pointer = node;
    node->next = before;
}

i32 main(void) {
    for (u8 i = 1; i < COUNT_NODES; ++i) {
        Node node = {
            .value = i,
            .next = &NODES[i],
        };
        NODES[i - 1] = node;
    }
    {
        Node node = {
            .value = 10,
            .next = NULL,
        };
        NODES[9] = node;
    }
    {
        List list = {
            .head = &NODES[0],
        };
        drop(&list, &NODES[0]);
        drop(&list, &NODES[3]);
        drop(&list, &NODES[9]);
        insert(&list, &NODES[1], &NODES[3]);
        insert(&list, &NODES[3], &NODES[9]);
        insert(&list, &NODES[9], &NODES[0]);
        Node* node = list.head;
        for (;;) {
            printf("%hhu\n", node->value);
            node = node->next;
            if (!node) {
                break;
            }
        }
    }
    printf("Done!\n");
    return EXIT_SUCCESS;
}
