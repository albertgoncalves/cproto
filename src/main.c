#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* NOTE: Forthcoming FIFO example via
 * `https://github.com/user-none/poddown/blob/master/src/tpool.c`.
 */

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

struct node {
    uint8_t      value;
    struct node* ptr;
};

typedef struct node node_t;

typedef struct linked_list {
    struct node* nodes;
} linked_list_t;

static void push(linked_list_t* list, uint8_t value) {
    node_t* next_node = malloc(sizeof(node_t));
    EXIT_IF(next_node == NULL);
    next_node->value = value;
    next_node->ptr   = list->nodes;
    list->nodes      = next_node;
}

static uint8_t pop(linked_list_t* list) {
    node_t* current_node = list->nodes;
    EXIT_IF(current_node == NULL);
    list->nodes   = current_node->ptr;
    uint8_t value = current_node->value;
    free(current_node);
    return value;
}

static uint8_t remove_at(linked_list_t* list, size_t index) {
    node_t* prev_node    = NULL;
    node_t* current_node = list->nodes;
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
        list->nodes = next_node;
    }
    uint8_t value = current_node->value;
    free(current_node);
    return value;
}

static void destroy(linked_list_t* list) {
    node_t* current_node = list->nodes;
    node_t* next_node;
    while (current_node != NULL) {
        next_node = current_node->ptr;
        free(current_node);
        current_node = next_node;
    }
}

static void print_list(linked_list_t* list) {
    node_t* current_node = list->nodes;
    printf("list.nodes  : [");
    while (current_node != NULL) {
        printf(" %hhu", current_node->value);
        current_node = current_node->ptr;
    }
    printf("]\n");
}

int main(void) {
    linked_list_t list = {.nodes = NULL};
    {
        for (size_t i = 0; i < 5; ++i) {
            uint8_t value = (uint8_t)i;
            printf("push()      : %hhu\n", value);
            push(&list, value);
        }
        print_list(&list);
        printf("\n");
    }
    {
        printf("remove_at() : %hhu\n", remove_at(&list, 1));
        print_list(&list);
        printf("\n");
    }
    {
        for (size_t i = 0; i < 2; ++i) {
            uint8_t value = pop(&list);
            printf("pop()       : %hhu\n", value);
        }
        print_list(&list);
    }
    destroy(&list);
    return 0;
}
