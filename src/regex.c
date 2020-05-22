#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define SIZE 16

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef struct State State;
typedef struct Stack Stack;

struct State {
    Stack* epsilon_transitions;
    State* transition;
    char   token;
    Bool   is_end;
};

struct Stack {
    State* states;
    u8     capacity;
    u8     index;
};

typedef struct {
    State* first;
    State* last;
} Link;

typedef struct {
    State states[SIZE];
    Stack stacks[SIZE];
    u8    state_index;
    u8    stack_index;
} Memory;

#define STATE_PUSH(state, memory) memory->states[memory->state_index++] = state
#define STATE_POP(memory)         memory->states[--memory->state_index]

static Stack* stack_new(Memory* memory, u8 capacity) {
    u8 index = (u8)(memory->state_index + capacity);
    if (SIZE <= index) {
        fprintf(stderr,
                "\n"
                "\033[1;31mError\033[0m@ \033[1mstack_new(..., %hu)\033[0m\n"
                "SIZE         : \033[1m%hu\033[0m\n"
                ".state_index : \033[1m%hu\033[0m\n",
                capacity,
                SIZE,
                memory->state_index);
        exit(1);
    }
    Stack* stack = &memory->stacks[memory->stack_index];
    stack->states = &memory->states[memory->state_index];
    stack->capacity = capacity;
    stack->index = 0;
    memory->state_index = index;
    return stack;
}

static void stack_push(Stack* stack, State state) {
    if (stack->index == stack->capacity) {
        fprintf(stderr,
                "\n"
                "\033[1;31mError\033[0m@ \033[1mstack_push(...)\033[0m\n"
                ".capacity : \033[1m%hu\033[0m\n"
                ".index    : \033[1m%hu\033[0m\n",
                stack->capacity,
                stack->index);
        exit(1);
    }
    stack->states[stack->index++] = state;
}

static State stack_pop(Stack* stack) {
    if (stack->index == 0) {
        fprintf(stderr,
                "\n"
                "\033[1;31mError\033[0m@ \033[1mstack_pop(...)\033[0m\n"
                ".index : \033[1m%hu\033[0m\n",
                stack->index);
        exit(1);
    }
    return stack->states[--stack->index];
}

int main(void) {
    printf("sizeof(Bool)   : %lu\n"
           "sizeof(State)  : %lu\n"
           "sizeof(Stack)  : %lu\n"
           "sizeof(Link)   : %lu\n"
           "sizeof(Memory) : %lu\n",
           sizeof(Bool),
           sizeof(State),
           sizeof(Stack),
           sizeof(Link),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    State   a = {
        .epsilon_transitions = NULL,
        .transition = NULL,
        .token = '\0',
        .is_end = FALSE,
    };
    STATE_PUSH(a, memory);
    State b = STATE_POP(memory);
    b.is_end = TRUE;
    Stack* stack = stack_new(memory, 2);
    stack_push(stack, b);
    stack_push(stack, b);
    stack_pop(stack);
    State c = stack_pop(stack);
    if (b.is_end) {
        printf("b.is_end : TRUE\n");
    } else {
        printf("b.is_end : FALSE\n");
    }
    if (c.is_end) {
        printf("c.is_end : TRUE\n");
    } else {
        printf("c.is_end : FALSE\n");
    }
    printf("%hu\n", memory->state_index);
    free(memory);
}
