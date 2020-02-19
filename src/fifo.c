#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* See `https://github.com/user-none/poddown/blob/master/src/tpool.c`. */

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
    struct node* first;
    struct node* last;
} fifo_queue_t;

static void push(fifo_queue_t* queue, T value) {
    node_t* next_node = malloc(sizeof(node_t));
    EXIT_IF(next_node == NULL);
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
}

static T pop(fifo_queue_t* queue) {
    node_t* current_node = queue->first;
    queue->first         = current_node->ptr;
    if (queue->first == queue->last) {
        queue->last = NULL;
    }
    T value = current_node->value;
    free(current_node);
    return value;
}

static void destroy(fifo_queue_t* queue) {
    node_t* current_node = queue->first;
    node_t* next_node;
    while (current_node != NULL) {
        next_node = current_node->ptr;
        free(current_node);
        current_node = next_node;
    }
}

static void print_queue(fifo_queue_t* queue) {
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
        for (size_t i = 0; i < 5; ++i) {
            T value = (T)i;
            printf("push()  : %hhu\n", value);
            push(&queue, value);
        }
        print_queue(&queue);
        printf("\n");
    }
    {
        for (size_t i = 0; i < 2; ++i) {
            printf("pop()   : %hhu\n", pop(&queue));
        }
        print_queue(&queue);
    }
    destroy(&queue);
    return 0;
}
