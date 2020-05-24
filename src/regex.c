#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define STATE_CAP 64
#define STACK_CAP 64

#define OP_CONCAT '.'

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef enum {
    TOKEN,
    SPLIT,
} StateType;

typedef struct State State;

typedef struct {
    State* state_a;
    State* state_b;
} Split;

typedef union {
    State* state;
    Split  split;
} Flow;

struct State {
    Flow      next;
    Flow      last;
    char      token;
    StateType type;
};

typedef struct {
    State  states[STATE_CAP];
    State* stack[STACK_CAP];
    State* stack_swap[STACK_CAP];
    u8     state_len;
} Memory;

typedef struct {
    State** states;
    u8      len;
} Stack;

static State* state_new(Memory* memory) {
    if (STATE_CAP <= memory->state_len) {
        exit(EXIT_FAILURE);
    }
    return &memory->states[memory->state_len++];
}

static State* get_nfa(Memory* memory, const char* postfix_expr) {
    Stack stack = {
        .states = memory->stack,
        .len = 0,
    };
    for (char token = *postfix_expr++; token != '\0'; token = *postfix_expr++)
    {
        switch (token) {
        case OP_CONCAT: {
            if (stack.len < 2) {
                exit(EXIT_FAILURE);
            }
            State* b = stack.states[--stack.len];
            State* a = stack.states[stack.len - 1];
            switch (a->type) {
            case TOKEN: {
                if (a->last.state == NULL) {
                    a->next.state = b;
                    a->last.state = b;
                } else {
                    switch (a->last.state->type) {
                    case TOKEN: {
                        a->last.state->next.state = b;
                        a->last.state->last.state = b;
                        a->last.state = b;
                        break;
                    }
                    case SPLIT: {
                        break;
                    }
                    }
                }
                break;
            }
            case SPLIT: {
                break;
            }
            }
            break;
        }
        default: {
            if (STACK_CAP <= stack.len) {
                exit(EXIT_FAILURE);
            }
            State* state = state_new(memory);
            state->type = TOKEN;
            state->token = token;
            stack.states[stack.len++] = state;
        }
        }
    }
    if (stack.len != 1) {
        exit(EXIT_FAILURE);
    }
    return stack.states[--stack.len];
}

static Bool get_match(Memory* memory, State* nfa, const char* string) {
    Stack stack = {
        .states = memory->stack,
        .len = 0,
    };
    Stack next = {
        .states = memory->stack_swap,
        .len = 0,
    };
    stack.states[stack.len++] = nfa;
    Bool end;
    for (char token = *string++; token != '\0'; token = *string++) {
        end = FALSE;
        for (u8 i = 0; i < stack.len; ++i) {
            State* state = stack.states[i];
            switch (state->type) {
            case TOKEN: {
                if (state->token == token) {
                    if (STACK_CAP <= next.len) {
                        exit(EXIT_FAILURE);
                    }
                    State* next_state = state->next.state;
                    if (next_state != NULL) {
                        next.states[next.len++] = next_state;
                    } else {
                        end = TRUE;
                    }
                }
                break;
            }
            case SPLIT: {
                break;
            }
            }
        }
        State** swap = stack.states;
        stack.states = next.states;
        stack.len = next.len;
        next.states = swap;
        next.len = 0;
    }
    return end;
}

int main(void) {
    printf("sizeof(State)     : %lu\n"
           "sizeof(StateType) : %lu\n"
           "sizeof(Memory)    : %lu\n",
           sizeof(State),
           sizeof(StateType),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    // const char* postfix_expr = "abc|*.d.";
    // const char* postfix_expr = "ab.c.";
    // const char* input = "abc";
    const char* postfix_expr = "ab.c.d.";
    const char* input = "abcd";
    Bool output = get_match(memory, get_nfa(memory, postfix_expr), input);
    printf("\"%s\" @ %s -> ", input, postfix_expr);
    switch (output) {
    case TRUE: {
        printf("TRUE\n");
        break;
    }
    case FALSE: {
        printf("FALSE\n");
        break;
    }
    }
    free(memory);
    return EXIT_SUCCESS;
}
