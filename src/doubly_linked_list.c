#include <stdio.h>
#include <stdlib.h>

typedef size_t usize;

typedef struct Node Node;

struct Node {
    Node*       next;
    Node*       prev;
    const char* value;
};

typedef struct {
    Node* first;
    Node* last;
} List;

#define SIZE 8

typedef struct {
    Node  buffer[SIZE];
    List  list;
    usize index;
} Memory;

static Node* alloc(Memory* memory, const char* value) {
    if (SIZE <= memory->index) {
        exit(EXIT_FAILURE);
    }
    Node* node = &memory->buffer[memory->index++];
    node->value = value;
    return node;
}

static void insert_first(List* list, Node* node) {
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->next = list->first;
        list->first->prev = node;
        list->first = node;
    }
}

static void insert_last(List* list, Node* node) {
    if (list->first == NULL) {
        list->first = node;
        list->last = node;
    } else {
        node->prev = list->last;
        list->last->next = node;
        list->last = node;
    }
}

static void print(List* list) {
    Node* node = list->first;
    printf("[");
    while (node != NULL) {
        printf(" \"%s\"", node->value);
        node = node->next;
    }
    printf(" ]\n");
}

static void print_rev(List* list) {
    Node* node = list->last;
    printf("[");
    while (node != NULL) {
        printf(" \"%s\"", node->value);
        node = node->prev;
    }
    printf(" ]\n");
}

#define INSERT_FIRST(x) insert_first(list, alloc(memory, x))
#define INSERT_LAST(x)  insert_last(list, alloc(memory, x))

int main(void) {
    printf("sizeof(Node)   : %zu\n"
           "sizeof(List)   : %zu\n"
           "sizeof(Memory) : %zu\n",
           sizeof(Node),
           sizeof(List),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    List*   list = &memory->list;
    INSERT_FIRST("is");
    INSERT_LAST("out");
    INSERT_FIRST("truth");
    INSERT_LAST("there");
    INSERT_LAST("!");
    INSERT_FIRST("the");
    print(&memory->list);
    print_rev(&memory->list);
    free(memory);
    return EXIT_SUCCESS;
}

#undef INSERT_FIRST
#undef INSERT_LAST
