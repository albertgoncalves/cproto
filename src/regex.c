#include <stdio.h>
#include <stdlib.h>

typedef unsigned char u8;

#define STATE_CAP 64
#define STACK_CAP 64

#define OP_CONCAT '.'
#define OP_EITHER '|'

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef enum {
    EPSILON = 0,
    TOKEN,
} StateType;

typedef struct State State;

struct State {
    State*    next;
    State*    next_split;
    StateType type;
    char      token;
    Bool      end;
};

typedef struct {
    State* first;
    State* last;
} Link;

typedef struct {
    State  states[STATE_CAP];
    Link   link_stack[STACK_CAP];
    State* state_stack_a[STACK_CAP];
    State* state_stack_b[STACK_CAP];
    State* state_stack_c[STACK_CAP];
    u8     state_len;
} Memory;

typedef struct {
    Link* links;
    u8    len;
} LinkStack;

typedef struct {
    State** states;
    u8      len;
} StateStack;

static State* state_new(Memory* memory) {
    if (STATE_CAP <= memory->state_len) {
        fprintf(stderr, "exit @ state_new(...)\n");
        exit(EXIT_FAILURE);
    }
    return &memory->states[memory->state_len++];
}

static void set_token(Memory* memory, LinkStack* stack, char token) {
    if (STACK_CAP <= stack->len) {
        fprintf(stderr, "exit @ set_token(...)\n");
        exit(EXIT_FAILURE);
    }
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.last->end = TRUE;
    link.first->type = TOKEN;
    link.first->token = token;
    link.first->next = link.last;
    stack->links[stack->len++] = link;
}

static void set_concat(LinkStack* stack) {
    if (stack->len < 2) {
        fprintf(stderr, "exit @ set_concat(...)\n");
        exit(EXIT_FAILURE);
    }
    Link b = stack->links[--stack->len];
    Link a = stack->links[--stack->len];
    a.last->next = b.first;
    a.last->end = FALSE;
    Link link = {
        .first = a.first,
        .last = b.last,
    };
    stack->links[stack->len++] = link;
}

static void set_either(Memory* memory, LinkStack* stack) {
    if (stack->len < 2) {
        fprintf(stderr, "exit @ set_either(...)\n");
        exit(EXIT_FAILURE);
    }
    Link b = stack->links[--stack->len];
    Link a = stack->links[--stack->len];
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.last->end = TRUE;
    link.first->next = a.first;
    link.first->next_split = b.first;
    a.last->next = link.last;
    b.last->next = link.last;
    a.last->end = FALSE;
    b.last->end = FALSE;
    stack->links[stack->len++] = link;
}

static Link get_nfa(Memory* memory, const char* postfix_expr) {
    LinkStack stack = {
        .links = memory->link_stack,
        .len = 0,
    };
    for (char token = *postfix_expr++; token != '\0'; token = *postfix_expr++)
    {
        switch (token) {
        case OP_CONCAT: {
            set_concat(&stack);
            break;
        }
        case OP_EITHER: {
            set_either(memory, &stack);
            break;
        }
        default: {
            set_token(memory, &stack, token);
        }
        }
    }
    if (stack.len != 1) {
        fprintf(stderr, "exit @ get_nfa(...)\n");
        exit(EXIT_FAILURE);
    }
    return stack.links[0];
}

static Bool get_match(Memory* memory, Link nfa, const char* string) {
    StateStack all_states = {
        .states = memory->state_stack_a,
        .len = 0,
    };
    StateStack buffer = {
        .states = memory->state_stack_b,
        .len = 0,
    };
    StateStack token_states = {
        .states = memory->state_stack_c,
        .len = 0,
    };
    all_states.states[all_states.len++] = nfa.first;
    for (char token = *string++; token != '\0'; token = *string++) {
        for (;;) {
            for (u8 i = 0; i < all_states.len; ++i) {
                State* state = all_states.states[i];
                switch (state->type) {
                case EPSILON: {
                    if (state->next != NULL) {
                        buffer.states[buffer.len++] = state->next;
                    }
                    if (state->next_split != NULL) {
                        buffer.states[buffer.len++] = state->next_split;
                    }
                    break;
                }
                case TOKEN: {
                    token_states.states[token_states.len++] = state;
                    break;
                }
                }
            }
            if (buffer.len == 0) {
                break;
            }
            State** swap = all_states.states;
            all_states.states = buffer.states;
            all_states.len = buffer.len;
            buffer.states = swap;
            buffer.len = 0;
        }
        if (token_states.len == 0) {
            return FALSE;
        }
        all_states.len = 0;
        Bool any_match = FALSE;
        do {
            State* state = token_states.states[--token_states.len];
            if (token == state->token) {
                if (any_match == FALSE) {
                    any_match = TRUE;
                }
                if (*string == '\0') {
                    if (state->next->end == TRUE) {
                        return TRUE;
                    }
                }
                if (state->next != NULL) {
                    all_states.states[all_states.len++] = state->next;
                }
                if (state->next_split != NULL) {
                    all_states.states[all_states.len++] = state->next_split;
                }
            }
        } while (0 < token_states.len);
        if (any_match == FALSE) {
            return FALSE;
        }
    }
    return FALSE;
}

#define TEST(postfix_expr, input, expected)                                  \
    if (get_match(memory, get_nfa(memory, postfix_expr), input) != expected) \
    {                                                                        \
        printf("\033[1;31mTest failed\033[0m @ "                             \
               "(\033[1m\"%s\", \"%s\"\033[0m)\n",                           \
               postfix_expr,                                                 \
               input);                                                       \
        exit(EXIT_FAILURE);                                                  \
    }

int main(void) {
    printf("sizeof(State)     : %lu\n"
           "sizeof(u8)        : %lu\n"
           "sizeof(StateType) : %lu\n"
           "sizeof(char)      : %lu\n"
           "sizeof(Bool)      : %lu\n"
           "sizeof(Link)      : %lu\n"
           "sizeof(Memory)    : %lu\n"
           "\n",
           sizeof(State),
           sizeof(u8),
           sizeof(StateType),
           sizeof(char),
           sizeof(Bool),
           sizeof(Link),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    if (memory == NULL) {
        fprintf(stderr, "exit @ main(...)\n");
        return EXIT_FAILURE;
    }
    TEST("ab.c.d.", "abcd", FALSE);
    TEST("ab|c.", "bc", TRUE);
    free(memory);
    return EXIT_SUCCESS;
}
