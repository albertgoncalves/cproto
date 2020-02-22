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
    node_t* first;
    node_t* last;
} fifo_queue_t;

typedef struct {
    T       value;
    error_t error;
} T_error_t;

static error_t push(fifo_queue_t* queue, const T value) {
    if (queue == NULL) {
        return true;
    }
    node_t* next_node = malloc(sizeof(node_t));
    if (next_node == NULL) {
        return true;
    }
    next_node->value = value;
    next_node->ptr   = NULL;
    if ((queue->first != NULL) && (queue->last == NULL)) {
        queue->first->ptr = next_node;
        queue->last       = next_node;
    } else if (queue->last != NULL) {
        queue->last->ptr = next_node;
        queue->last      = next_node;
    } else {
        queue->first = next_node;
    }
    return false;
}

static T_error_t pop(fifo_queue_t* queue) {
    T_error_t result;
    if (queue == NULL || queue->first == NULL) {
        result.error = true;
        return result;
    }
    node_t* current_node = queue->first;
    queue->first         = current_node->ptr;
    if (queue->first == queue->last) {
        queue->last = NULL;
    }
    result.value = current_node->value;
    result.error = false;
    free(current_node);
    return result;
}

static void destroy(fifo_queue_t* queue) {
    if (queue == NULL) {
        return;
    }
    node_t* current_node = queue->first;
    node_t* next_node;
    while (current_node != NULL) {
        next_node = current_node->ptr;
        free(current_node);
        current_node = next_node;
    }
    queue->first = NULL;
    queue->last  = NULL;
}

static void print_queue(const fifo_queue_t* queue) {
    if (queue == NULL) {
        return;
    }
    node_t* current_node = queue->first;
    printf("queue[] : [");
    while (current_node != NULL) {
        printf(" %hhu", current_node->value);
        current_node = current_node->ptr;
    }
    printf("]\n");
}

int main(void) {
    fifo_queue_t queue = {.first = NULL, .last = NULL};
    {
        for (size_t i = 0; i < 10; ++i) {
            const T value = (T)i;
            printf("push()  : %hhu\n", value);
            EXIT_IF(push(&queue, value));
        }
        print_queue(&queue);
        printf("\n");
    }
    {
        for (T_error_t r = pop(&queue); !r.error; r = pop(&queue)) {
            printf("pop()   : %hhu\n", r.value);
        }
        print_queue(&queue);
    }
    EXIT_IF(push(&queue, (T)11));
    EXIT_IF(push(&queue, (T)12));
    print_queue(&queue);
    destroy(&queue);
    return 0;
}
