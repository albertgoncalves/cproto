#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

#define STATE_CAP 22
#define STACK_CAP 12

#define OP_CONCAT       '.'
#define OP_EITHER       '|'
#define OP_ZERO_OR_ONE  '?'
#define OP_ZERO_OR_MANY '*'
#define OP_ONE_OR_MANY  '+'

#define PRINT_ERROR \
    fprintf(stderr, "\033[1;31mError\033[0m @ \033[1m%s\033[0m\n", __func__)

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
    State* state_stack_d[STACK_CAP];
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
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    State* state = &memory->states[memory->state_len++];
    state->next = NULL;
    state->next_split = NULL;
    state->type = EPSILON;
    state->token = '\0';
    state->end = FALSE;
    return state;
}

static void set_token(Memory* memory, LinkStack* stack, char token) {
    if (STACK_CAP <= stack->len) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.first->type = TOKEN;
    link.first->token = token;
    link.first->next = link.last;
    stack->links[stack->len++] = link;
}

static void set_concat(LinkStack* stack) {
    if (stack->len < 2) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link b = stack->links[--stack->len];
    Link a = stack->links[--stack->len];
    a.last->next = b.first;
    Link link = {
        .first = a.first,
        .last = b.last,
    };
    stack->links[stack->len++] = link;
}

static void set_either(Memory* memory, LinkStack* stack) {
    if (stack->len < 2) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link b = stack->links[--stack->len];
    Link a = stack->links[--stack->len];
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.first->next = a.first;
    link.first->next_split = b.first;
    a.last->next = link.last;
    b.last->next = link.last;
    stack->links[stack->len++] = link;
}

static void set_zero_or_one(Memory* memory, LinkStack* stack) {
    if (stack->len < 1) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link a = stack->links[--stack->len];
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.first->next = link.last;
    link.first->next_split = a.first;
    a.last->next = link.last;
    stack->links[stack->len++] = link;
}

static void set_zero_or_many(Memory* memory, LinkStack* stack) {
    if (stack->len < 1) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link a = stack->links[--stack->len];
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    /* NOTE: To simplify the implementation, looping pointers should *always*
     * be assigned to `State.next_split`. This way, we can simply follow
     * `State.next` to find the terminal `State`.
     */
    link.first->next = link.last;
    link.first->next_split = a.first;
    a.last->next = link.last;
    a.last->next_split = a.first;
    stack->links[stack->len++] = link;
}

static void set_one_or_many(Memory* memory, LinkStack* stack) {
    if (stack->len < 1) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link a = stack->links[--stack->len];
    Link link = {
        .first = state_new(memory),
        .last = state_new(memory),
    };
    link.first->next = a.first;
    a.last->next = link.last;
    a.last->next_split = a.first;
    stack->links[stack->len++] = link;
}

static Link get_nfa(Memory* memory, const char* postfix_expr) {
    /* NOTE: See `https://swtch.com/~rsc/regexp/regexp1.html`. */
    memory->state_len = 0;
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
        case OP_ZERO_OR_ONE: {
            set_zero_or_one(memory, &stack);
            break;
        }
        case OP_ZERO_OR_MANY: {
            set_zero_or_many(memory, &stack);
            break;
        }
        case OP_ONE_OR_MANY: {
            set_one_or_many(memory, &stack);
            break;
        }
        default: {
            set_token(memory, &stack, token);
        }
        }
    }
    if (stack.len != 1) {
        PRINT_ERROR;
        exit(EXIT_FAILURE);
    }
    Link link = stack.links[0];
    link.last->end = TRUE;
    return link;
}

#define PUSH(stack, state)                 \
    {                                      \
        if (STACK_CAP <= stack.len) {      \
            PRINT_ERROR;                   \
            exit(EXIT_FAILURE);            \
        }                                  \
        stack.states[stack.len++] = state; \
    }

static Bool get_empty_match(Memory* memory, State* state) {
    if (state->type != EPSILON) {
        return FALSE;
    }
    StateStack stack = {
        .states = memory->state_stack_a,
        .len = 0,
    };
    PUSH(stack, state);
    while (stack.len != 0) {
        State* last_state = stack.states[--stack.len];
        if (last_state->type == EPSILON) {
            if (last_state->end) {
                return TRUE;
            }
            if (last_state->next != NULL) {
                PUSH(stack, last_state->next);
            }
            if (last_state->next_split != NULL) {
                PUSH(stack, last_state->next_split);
            }
        }
    }
    return FALSE;
}

#undef PUSH

#define PUSH(stack, state)                 \
    {                                      \
        if (STACK_CAP <= stack.len) {      \
            PRINT_ERROR;                   \
            exit(EXIT_FAILURE);            \
        }                                  \
        stack.states[stack.len++] = state; \
    }

static Bool get_match(Memory* memory, Link link, const char* string) {
    char token = *string;
    if (token == '\0') {
        return get_empty_match(memory, link.first);
    }
    StateStack stack_all = {
        .states = memory->state_stack_a,
        .len = 0,
    };
    StateStack stack_buffer = {
        .states = memory->state_stack_b,
        .len = 0,
    };
    StateStack stack_tokens = {
        .states = memory->state_stack_c,
        .len = 0,
    };
    StateStack stack_visited = {
        .states = memory->state_stack_d,
        .len = 0,
    };
    PUSH(stack_all, link.first);
    while (token != '\0') {
        for (;;) {
            for (u8 i = 0; i < stack_all.len; ++i) {
                State* state = stack_all.states[i];
                Bool   visited = FALSE;
                for (u8 j = 0; j < stack_visited.len; ++j) {
                    if (stack_visited.states[j] == state) {
                        visited = TRUE;
                        break;
                    }
                }
                if (visited) {
                    continue;
                }
                switch (state->type) {
                case EPSILON: {
                    if (state->next != NULL) {
                        PUSH(stack_buffer, state->next);
                    }
                    if (state->next_split != NULL) {
                        PUSH(stack_buffer, state->next_split);
                    }
                    break;
                }
                case TOKEN: {
                    PUSH(stack_tokens, state);
                    break;
                }
                }
                PUSH(stack_visited, state);
            }
            if (stack_buffer.len == 0) {
                break;
            }
            State** swap = stack_all.states;
            stack_all.states = stack_buffer.states;
            stack_all.len = stack_buffer.len;
            stack_buffer.states = swap;
            stack_buffer.len = 0;
        }
        if (stack_tokens.len == 0) {
            return FALSE;
        }
        stack_all.len = 0;
        stack_visited.len = 0;
        char peek = *++string;
        for (u8 i = 0; i < stack_tokens.len; ++i) {
            State* state = stack_tokens.states[i];
            if ((token == state->token) && (state->next != NULL)) {
                if (peek == '\0') {
                    /* NOTE: Looping paths should *always* be inscribed onto
                     * `State.next_split`; assuming that is the case, it is
                     * safe to search for `State.end` by following
                     * `State.next`.
                     */
                    State* last_state = state->next;
                    while (last_state != NULL) {
                        if (last_state->type == EPSILON) {
                            if (last_state->end) {
                                return TRUE;
                            }
                            last_state = last_state->next;
                        } else {
                            break;
                        }
                    }
                } else {
                    PUSH(stack_all, state->next);
                }
            }
        }
        if (stack_all.len == 0) {
            return FALSE;
        }
        stack_tokens.len = 0;
        token = peek;
    }
    return FALSE;
}

#undef PUSH

static u8 TESTS_PASSED = 0;
static u8 TESTS_FAILED = 0;

#define TEST(postfix_expr, input, expected)                            \
    {                                                                  \
        if (get_match(memory, get_nfa(memory, postfix_expr), input) == \
            expected) {                                                \
            ++TESTS_PASSED;                                            \
        } else {                                                       \
            printf("\033[1;31mTest failed\033[0m @ "                   \
                   "(\033[1m\"%s\", \"%s\"\033[0m)\n",                 \
                   postfix_expr,                                       \
                   input);                                             \
            ++TESTS_FAILED;                                            \
        }                                                              \
    }

int main(void) {
    printf("sizeof(State)     : %zu\n"
           "sizeof(u8)        : %zu\n"
           "sizeof(StateType) : %zu\n"
           "sizeof(char)      : %zu\n"
           "sizeof(Bool)      : %zu\n"
           "sizeof(Link)      : %zu\n"
           "sizeof(Memory)    : %zu\n"
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
        PRINT_ERROR;
        return EXIT_FAILURE;
    }
    TEST("ab.c.d.", "abcd", TRUE);
    TEST("ab.c.d.", "abdc", FALSE);
    TEST("ab.c.d.", "bbcd", FALSE);
    TEST("ab.c.d.", "abc", FALSE);
    TEST("ab.c.d.", "bcd", FALSE);
    TEST("ab|c.", "ac", TRUE);
    TEST("ab|c.", "bc", TRUE);
    TEST("ab|c.", "ab", FALSE);
    TEST("ab|c.", "ba", FALSE);
    TEST("ab|c.", "a", FALSE);
    TEST("ab|c.", "b", FALSE);
    TEST("ab|c.", "c", FALSE);
    TEST("abc|*.d.", "abcd", TRUE);
    TEST("abc|*.d.", "abd", TRUE);
    TEST("abc|*.d.", "acd", TRUE);
    TEST("abc|*.d.", "abbd", TRUE);
    TEST("abc|*.d.", "accd", TRUE);
    TEST("abc|*.d.", "bcd", FALSE);
    TEST("abc|*.d.", "bd", FALSE);
    TEST("abc|*.d.", "cd", FALSE);
    TEST("abc|*.d.", "bbd", FALSE);
    TEST("abc|*.d.", "ccd", FALSE);
    TEST("abc|*.d.", "abc", FALSE);
    TEST("abc|*.d.", "ab", FALSE);
    TEST("abc|*.d.", "ac", FALSE);
    TEST("abc|*.d.", "abb", FALSE);
    TEST("abc|*.d.", "acc", FALSE);
    TEST("ab|", "a", TRUE);
    TEST("ab|", "b", TRUE);
    TEST("ab|cd|.", "ac", TRUE);
    TEST("ab|cd|.ef|.", "ace", TRUE);
    TEST("ab|cd|.ef|.", "bdf", TRUE);
    TEST("ab|*c.", "c", TRUE);
    TEST("ab|*c.", "ac", TRUE);
    TEST("ab|*c.", "bc", TRUE);
    TEST("ab|*c.", "abc", TRUE);
    TEST("ab|*c.", "bac", TRUE);
    TEST("ab|*c.", "aabbc", TRUE);
    TEST("ab|*c.", "cc", FALSE);
    TEST("ab|*c.", "cab", FALSE);
    TEST("ab|*c.", "cabc", FALSE);
    TEST("ab|*c.", "cba", FALSE);
    TEST("ab|*c.", "cc", FALSE);
    TEST("ab|+c.", "c", FALSE);
    TEST("ab|+c.", "ac", TRUE);
    TEST("ab|+c.", "bc", TRUE);
    TEST("ab|+c.", "abc", TRUE);
    TEST("ab|+c.", "bac", TRUE);
    TEST("ab|+c.", "aabbc", TRUE);
    TEST("ab|+c.", "cc", FALSE);
    TEST("ab|+c.", "cab", FALSE);
    TEST("ab|+c.", "cabc", FALSE);
    TEST("ab|+c.", "cba", FALSE);
    TEST("ab|+c.", "cc", FALSE);
    TEST("ab|?c.", "c", TRUE);
    TEST("ab|?c.", "ac", TRUE);
    TEST("ab|?c.", "bc", TRUE);
    TEST("ab|?c.", "abc", FALSE);
    TEST("ab|?c.", "bac", FALSE);
    TEST("ab|?c.", "aabbc", FALSE);
    TEST("ab|?c.", "cc", FALSE);
    TEST("ab|?c.", "cab", FALSE);
    TEST("ab|?c.", "cabc", FALSE);
    TEST("ab|?c.", "cba", FALSE);
    TEST("ab|?c.", "cc", FALSE);
    TEST("ab.c.", "abc", TRUE);
    TEST("ab.c.", "abcd", FALSE);
    TEST("a", "", FALSE);
    TEST("a", "a", TRUE);
    TEST("a", "aaaaaaa", FALSE);
    TEST("a*", "", TRUE);
    TEST("a*", "b", FALSE);
    TEST("a*", "a", TRUE);
    TEST("a*", "aaaaaaa", TRUE);
    TEST("a+", "", FALSE);
    TEST("a+", "b", FALSE);
    TEST("a+", "a", TRUE);
    TEST("a+", "aaaaaaa", TRUE);
    TEST("aa*.", "", FALSE);
    TEST("aa*.", "a", TRUE);
    TEST("aa*.", "aaaaaaa", TRUE);
    TEST("aa*.", "baaaaaa", FALSE);
    TEST("aa*.", "aaaaaab", FALSE);
    TEST("aa*.", "aaabaab", FALSE);
    TEST("aa?.", "", FALSE);
    TEST("aa?.", "a", TRUE);
    TEST("aa?.", "aa", TRUE);
    TEST("aa?.", "aaa", FALSE);
    TEST("aa.a*.", "", FALSE);
    TEST("aa.a*.", "a", FALSE);
    TEST("aa.a*.", "aa", TRUE);
    TEST("aa.a*.", "aaaaaaa", TRUE);
    TEST("aa.a*.", "baaaaaa", FALSE);
    TEST("aa.a*.", "aaaaaab", FALSE);
    TEST("aa.a*.", "aaabaaa", FALSE);
    TEST("a*b*.", "", TRUE);
    TEST("a*b*.", "a", TRUE);
    TEST("a*b*.", "b", TRUE);
    TEST("a*b*.", "ab", TRUE);
    TEST("a*b*.", "aa", TRUE);
    TEST("a*b*.", "bb", TRUE);
    TEST("a*b*.", "aab", TRUE);
    TEST("a*b*.", "abb", TRUE);
    TEST("a*b*.", "c", FALSE);
    TEST("a*b*.", "ca", FALSE);
    TEST("a*b*.", "cb", FALSE);
    TEST("a*b*.", "cab", FALSE);
    TEST("a*b*.", "caa", FALSE);
    TEST("a*b*.", "cbb", FALSE);
    TEST("a*b*.", "caab", FALSE);
    TEST("a*b*.", "cabb", FALSE);
    TEST("a*b*.", "ac", FALSE);
    TEST("a*b*.", "bc", FALSE);
    TEST("a*b*.", "abc", FALSE);
    TEST("a*b*.", "aac", FALSE);
    TEST("a*b*.", "bbc", FALSE);
    TEST("a*b*.", "aabc", FALSE);
    TEST("a*b*.", "abbc", FALSE);
    TEST("abc|*d.|", "a", TRUE);
    TEST("abc|*d.|", "bd", TRUE);
    TEST("abc|*d.|", "cd", TRUE);
    TEST("abc|*d.|", "bbd", TRUE);
    TEST("abc|*d.|", "ccd", TRUE);
    TEST("abc|*d.|", "bbcd", TRUE);
    TEST("abc|*d.|", "bccd", TRUE);
    TEST("abc|*d.|", "bcd", TRUE);
    TEST("abc|*d.|", "abcd", FALSE);
    TEST("abc|*d.|", "", FALSE);
    TEST("a*b|", "", TRUE);
    TEST("a*b|", "a", TRUE);
    TEST("a*b|", "aa", TRUE);
    TEST("a*b|", "b", TRUE);
    TEST("a+b|", "", FALSE);
    TEST("a+b|", "a", TRUE);
    TEST("a+b|", "aa", TRUE);
    TEST("a+b|", "b", TRUE);
    TEST("ab*|", "", TRUE);
    TEST("ab*|", "a", TRUE);
    TEST("ab*|", "b", TRUE);
    TEST("ab*|", "bb", TRUE);
    TEST("ca*b|.", "c", TRUE);
    TEST("ca*b|.", "ca", TRUE);
    TEST("ca*b|.", "caa", TRUE);
    TEST("ca*b|.", "cb", TRUE);
    TEST("ca*b|.", "", FALSE);
    TEST("ca*b|.", "a", FALSE);
    TEST("ca*b|.", "aa", FALSE);
    TEST("ca*b|.", "b", FALSE);
    TEST("ca?b|.", "c", TRUE);
    TEST("ca?b|.", "ca", TRUE);
    TEST("ca?b|.", "caa", FALSE);
    TEST("ca?b|.", "cb", TRUE);
    TEST("ca?b|.", "", FALSE);
    TEST("ca?b|.", "a", FALSE);
    TEST("ca?b|.", "aa", FALSE);
    TEST("ca?b|.", "b", FALSE);
    TEST("cab*|*.", "c", TRUE);
    TEST("cab*|*.", "ca", TRUE);
    TEST("cab*|*.", "cb", TRUE);
    TEST("cab*|*.", "cbb", TRUE);
    TEST("cab*|*.", "cabbabbabb", TRUE);
    TEST("cab*|*.", "caabbabbabb", TRUE);
    TEST("cab*|*.", "", FALSE);
    TEST("cab*|*.", "a", FALSE);
    TEST("cab*|*.", "b", FALSE);
    TEST("cab*|*.", "bb", FALSE);
    TEST("cab*|*.", "abbabbabb", FALSE);
    TEST("cab*|*.", "aabbabbabb", FALSE);
    TEST("abcdef|||||", "a", TRUE);
    TEST("abcdef|||||", "b", TRUE);
    TEST("abcdef|||||", "c", TRUE);
    TEST("abcdef|||||", "d", TRUE);
    TEST("abcdef|||||", "e", TRUE);
    TEST("abcdef|||||", "f", TRUE);
    TEST("abcdef|||||", "g", FALSE);
    TEST("abcdef|||||", "h", FALSE);
    TEST("abcdef|||||", "", FALSE);
    if (0 < TESTS_FAILED) {
        printf("\n");
    }
    if (TESTS_PASSED == 1) {
        printf("\033[1;36m  1 test  passed\033[0m\n");
    } else if (1 < TESTS_PASSED) {
        printf("\033[1;36m%3hhu tests passed\033[0m\n", TESTS_PASSED);
    }
    if (TESTS_FAILED == 1) {
        printf("\033[1;33m  1 test  failed\033[0m\n");
    } else if (1 < TESTS_FAILED) {
        printf("\033[1;33m%3hhu tests failed\033[0m\n", TESTS_FAILED);
    }
    free(memory);
    return EXIT_SUCCESS;
}
