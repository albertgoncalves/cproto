#include <stdbool.h>
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

typedef uint8_t T;
typedef bool    error_t;

typedef struct node {
    T            value;
    struct node* ptr;
} node_t;

typedef struct {
    node_t* head;
} linked_list_t;

typedef struct {
    T       value;
    error_t error;
} T_error_t;

static error_t push(linked_list_t* list, const T value) {
    if (list == NULL) {
        return true;
    }
    node_t* next_node = malloc(sizeof(node_t));
    if (next_node == NULL) {
        return true;
    }
    next_node->value = value;
    next_node->ptr   = list->head;
    list->head       = next_node;
    return false;
}

static T_error_t pop(linked_list_t* list) {
    T_error_t result;
    if ((list == NULL) || (list->head == NULL)) {
        result.error = true;
        return result;
    }
    node_t* current_node = list->head;
    list->head           = current_node->ptr;
    result.value         = current_node->value;
    result.error         = false;
    free(current_node);
    return result;
}

static T_error_t pop_at(linked_list_t* list, const size_t index) {
    T_error_t result;
    if ((list == NULL) || (list->head == NULL)) {
        result.error = true;
        return result;
    }
    node_t* prev_node    = NULL;
    node_t* current_node = list->head;
    for (size_t i = 0; i < index; ++i) {
        if (current_node->ptr == NULL) {
            result.error = true;
            return result;
        }
        prev_node    = current_node;
        current_node = current_node->ptr;
    }
    if (prev_node == NULL) {
        list->head = current_node->ptr;
    } else {
        prev_node->ptr = current_node->ptr;
    }
    result.value = current_node->value;
    result.error = false;
    free(current_node);
    return result;
}

static void destroy(linked_list_t* list) {
    if (list == NULL) {
        return;
    }
    node_t* current_node = list->head;
    node_t* next_node;
    while (current_node != NULL) {
        next_node = current_node->ptr;
        free(current_node);
        current_node = next_node;
    }
    list->head = NULL;
}

static void print_list(const linked_list_t* list) {
    if (list == NULL) {
        return;
    }
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
            const T value = (T)i;
            printf("push()   : %hhu\n", value);
            EXIT_IF(push(&list, value));
        }
        print_list(&list);
        printf("\n");
    }
    {
        T_error_t r = pop_at(&list, 1);
        if (!r.error) {
            printf("pop_at() : %hhu\n", r.value);
        }
        print_list(&list);
        printf("\n");
    }
    {
        for (T_error_t r = pop(&list); !r.error; r = pop(&list)) {
            printf("pop()    : %hhu\n", r.value);
        }
        print_list(&list);
    }
    destroy(&list);
    return 0;
}
