#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BOLD_PURPLE    "\033[1;35m"
#define BOLD           "\033[1m"
#define BOLD_UNDERLINE "\033[1;4m"
#define CLOSE          "\033[0m"

#define EXIT_IF(condition)                                                \
    if (condition) {                                                      \
        fprintf(stderr, "<%s%s%s>.%s%s%s() @%sline %d%s: %sExit%s", BOLD, \
                __FILE__, CLOSE, BOLD, __func__, CLOSE, BOLD_UNDERLINE,   \
                __LINE__, CLOSE, BOLD_PURPLE, CLOSE);                     \
        exit(1);                                                          \
    }

#define T uint8_t

typedef struct node {
    T            value;
    struct node* ptr;
} node_t;

typedef struct {
    struct node* head;
} linked_list_t;

static void push(linked_list_t* list, T value) {
    node_t* next_node = malloc(sizeof(node_t));
    EXIT_IF(next_node == NULL);
    next_node->value = value;
    next_node->ptr   = list->head;
    list->head       = next_node;
}

static T pop(linked_list_t* list) {
    node_t* current_node = list->head;
    EXIT_IF(current_node == NULL);
    list->head = current_node->ptr;
    T value    = current_node->value;
    free(current_node);
    return value;
}

static T pop_at(linked_list_t* list, size_t index) {
    node_t* prev_node    = NULL;
    node_t* current_node = list->head;
    node_t* next_node    = current_node->ptr;
    for (size_t i = 0; i < index; ++i) {
        EXIT_IF(next_node == NULL);
        prev_node    = current_node;
        current_node = next_node;
        next_node    = next_node->ptr;
    }
    if (prev_node != NULL) {
        prev_node->ptr = next_node;
    } else {
        list->head = next_node;
    }
    T value = current_node->value;
    free(current_node);
    return value;
}

static void destroy(linked_list_t* list) {
    node_t* current_node = list->head;
    node_t* next_node;
    while (current_node != NULL) {
        next_node = current_node->ptr;
        free(current_node);
        current_node = next_node;
    }
}

static void print_list(linked_list_t* list) {
    node_t* current_node = list->head;
    printf("list[]   : [");
    while (current_node != NULL) {
        printf(" %hhu", current_node->value);
        current_node = current_node->ptr;
    }
    printf("]\n");
}

int main(void) {
    linked_list_t list = {.head = NULL};
    {
        for (size_t i = 0; i < 5; ++i) {
            T value = (T)i;
            printf("push()   : %hhu\n", value);
            push(&list, value);
        }
        print_list(&list);
        printf("\n");
    }
    {
        printf("pop_at() : %hhu\n", pop_at(&list, 1));
        print_list(&list);
        printf("\n");
    }
    {
        for (size_t i = 0; i < 2; ++i) {
            printf("pop()    : %hhu\n", pop(&list));
        }
        print_list(&list);
    }
    destroy(&list);
    return 0;
}
