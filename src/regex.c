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
    State states[STATE_CAP];
    Link  stack_a[STACK_CAP];
    Link  stack_b[STACK_CAP];
    Link  stack_c[STACK_CAP];
    u8    state_len;
} Memory;

typedef struct {
    Link* links;
    u8    len;
} Stack;

static State* state_new(Memory* memory) {
    if (STATE_CAP <= memory->state_len) {
        exit(EXIT_FAILURE);
    }
    return &memory->states[memory->state_len++];
}

static void set_token(Memory* memory, Stack* stack, char token) {
    if (STACK_CAP <= stack->len) {
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

static void set_concat(Stack* stack) {
    if (stack->len < 2) {
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

static Link get_nfa(Memory* memory, const char* postfix_expr) {
    Stack stack = {
        .links = memory->stack_a,
        .len = 0,
    };
    for (char token = *postfix_expr++; token != '\0'; token = *postfix_expr++)
    {
        switch (token) {
        case OP_CONCAT: {
            set_concat(&stack);
            break;
        }
        default: {
            set_token(memory, &stack, token);
        }
        }
    }
    if (stack.len != 1) {
        exit(EXIT_FAILURE);
    }
    return stack.links[0];
}

static Bool get_match(Memory* memory, Link nfa, const char* string) {
    Stack mixed_states = {
        .links = memory->stack_a,
        .len = 0,
    };
    Stack buffer = {
        .links = memory->stack_b,
        .len = 0,
    };
    Stack nfa_tokens = {
        .links = memory->stack_c,
        .len = 0,
    };
    mixed_states.links[mixed_states.len++] = nfa;
    for (char token = *string++; token != '\0'; token = *string++) {
        for (;;) {
            for (u8 i = 0; i < mixed_states.len; ++i) {
                State* state = mixed_states.links[i].first;
                switch (state->type) {
                case EPSILON: {
                    if (state->next != NULL) {
                        buffer.links[buffer.len++].first = state->next;
                    }
                    if (state->next_split != NULL) {
                        buffer.links[buffer.len++].first = state->next_split;
                    }
                    break;
                }
                case TOKEN: {
                    nfa_tokens.links[nfa_tokens.len++].first = state;
                    break;
                }
                }
            }
            if (buffer.len == 0) {
                break;
            }
            Link* swap = mixed_states.links;
            mixed_states.links = buffer.links;
            mixed_states.len = buffer.len;
            buffer.links = swap;
            buffer.len = 0;
        }
        if (nfa_tokens.len == 0) {
            return FALSE;
        }
        do {
            State* state = nfa_tokens.links[--nfa_tokens.len].first;
            if (token == state->token) {
                if (*string == '\0') {
                    return state->next->end;
                }
                if (state->next != NULL) {
                    mixed_states.links[mixed_states.len++].first = state->next;
                }
                if (state->next_split != NULL) {
                    mixed_states.links[mixed_states.len++].first =
                        state->next_split;
                }
            }
        } while (0 < nfa_tokens.len);
    }
    return FALSE;
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
        return EXIT_FAILURE;
    }
    const char* postfix_expr = "ab.c.d.";
    const char* input = "abcd";
    Link        nfa = get_nfa(memory, postfix_expr);
    Bool        output = get_match(memory, nfa, input);
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
