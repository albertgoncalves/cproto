#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://swtch.com/~rsc/regexp/regexp2.html`.

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

#define CAP_LISTS  32
#define CAP_LABELS 32
#define CAP_INSTS  32

typedef struct {
    const char* chars;
    u8          len;
} String;

typedef enum {
    LIST_CHAR = 0,
    LIST_CONCAT,
    LIST_OR,
    LIST_ZERO_OR_ONE,
    LIST_ZERO_OR_MANY,
    LIST_ONE_OR_MANY,
} ListTag;

typedef struct List List;

typedef union {
    List* as_list[2];
    char  as_char;
} ListOp;

struct List {
    ListOp  op;
    ListTag tag;
};

typedef enum {
    INST_MATCH = 0,
    INST_CHAR,
    INST_JMP,
    INST_SPLIT,
    COUNT_INST_TAG,
} InstTag;

typedef union {
    u8   as_label[2];
    char as_char;
} InstOp;

typedef struct {
    InstOp  op;
    InstTag tag;
} Inst;

typedef struct {
    u8 program;
    u8 string;
} Index;

typedef struct {
    List  lists[CAP_LISTS];
    u8    len_lists;
    u8    labels[CAP_LABELS];
    Inst  insts[CAP_INSTS];
    Index index;
} Memory;

#define EXIT_IF(condition)         \
    if (condition) {               \
        fprintf(stderr,            \
                "%s:%s:%d `%s`\n", \
                __FILE__,          \
                __func__,          \
                __LINE__,          \
                #condition);       \
        exit(EXIT_FAILURE);        \
    }

#define PARSE_ERROR(string, i)               \
    {                                        \
        fprintf(stderr,                      \
                "%s:%s:%d\n\"%.*s\":%hhu\n", \
                __FILE__,                    \
                __func__,                    \
                __LINE__,                    \
                string.len,                  \
                string.chars,                \
                i + 1);                      \
        exit(EXIT_FAILURE);                  \
    }

#define TO_STRING(literal)          \
    ((String){                      \
        .chars = literal,           \
        .len = sizeof(literal) - 1, \
    })

static List* alloc_list(Memory* memory) {
    EXIT_IF(CAP_LISTS <= memory->len_lists);
    return &memory->lists[memory->len_lists++];
}

static List* parse(Memory* memory, String string, u8 index) {
    List* prev_list = NULL;
    for (u8 i = index; i < string.len; ++i) {
        switch (string.chars[i]) {
        case '|': {
            List* list = alloc_list(memory);
            list->tag = LIST_OR;
            list->op.as_list[0] = prev_list;
            list->op.as_list[1] = parse(memory, string, i + 1);
            return list;
        }
        case '?': {
            if (prev_list) {
                switch (prev_list->tag) {
                case LIST_CHAR:
                case LIST_CONCAT:
                case LIST_OR: {
                    break;
                }
                case LIST_ZERO_OR_ONE:
                case LIST_ZERO_OR_MANY:
                case LIST_ONE_OR_MANY: {
                    PARSE_ERROR(string, i);
                }
                }
            }
            List* list = alloc_list(memory);
            list->tag = LIST_ZERO_OR_ONE;
            list->op.as_list[0] = prev_list;
            prev_list = list;
            break;
        }
        case '*': {
            if (prev_list) {
                switch (prev_list->tag) {
                case LIST_CHAR:
                case LIST_CONCAT:
                case LIST_OR: {
                    break;
                }
                case LIST_ZERO_OR_ONE:
                case LIST_ZERO_OR_MANY:
                case LIST_ONE_OR_MANY: {
                    PARSE_ERROR(string, i);
                }
                }
            }
            List* list = alloc_list(memory);
            list->tag = LIST_ZERO_OR_MANY;
            list->op.as_list[0] = prev_list;
            prev_list = list;
            break;
        }
        case '+': {
            if (prev_list) {
                switch (prev_list->tag) {
                case LIST_CHAR:
                case LIST_CONCAT:
                case LIST_OR: {
                    break;
                }
                case LIST_ZERO_OR_ONE:
                case LIST_ZERO_OR_MANY:
                case LIST_ONE_OR_MANY: {
                    PARSE_ERROR(string, i);
                }
                }
            }
            List* list = alloc_list(memory);
            list->tag = LIST_ONE_OR_MANY;
            list->op.as_list[0] = prev_list;
            prev_list = list;
            break;
        }
        default: {
            List* list = alloc_list(memory);
            if (prev_list && (prev_list->tag == LIST_CHAR)) {
                list->tag = LIST_CONCAT;
                list->op.as_list[0] = prev_list;
                list->op.as_list[1] = parse(memory, string, i);
                return list;
            } else {
                list->tag = LIST_CHAR;
                list->op.as_char = string.chars[i];
                prev_list = list;
            }
        }
        }
    }
    return prev_list;
}

#define INDENT(n)                    \
    {                                \
        for (u8 i = 0; i < n; ++i) { \
            printf(" ");             \
        }                            \
    }

#define SHOW_INCR 2

void show_expr(List*, u8);
void show_expr(List* list, u8 n) {
    if (!list) {
        return;
    }
    switch (list->tag) {
    case LIST_CHAR: {
        INDENT(n);
        printf("'%c'\n", list->op.as_char);
        break;
    }
    case LIST_CONCAT: {
        show_expr(list->op.as_list[1], n + SHOW_INCR);
        INDENT(n);
        printf(".\n");
        show_expr(list->op.as_list[0], n + SHOW_INCR);
        break;
    }
    case LIST_OR: {
        show_expr(list->op.as_list[1], n + SHOW_INCR);
        INDENT(n);
        printf("|\n");
        show_expr(list->op.as_list[0], n + SHOW_INCR);
        break;
    }
    case LIST_ZERO_OR_ONE: {
        show_expr(list->op.as_list[0], n + SHOW_INCR);
        INDENT(n);
        printf("?\n");
        break;
    }
    case LIST_ZERO_OR_MANY: {
        show_expr(list->op.as_list[0], n + SHOW_INCR);
        INDENT(n);
        printf("*\n");
        break;
    }
    case LIST_ONE_OR_MANY: {
        show_expr(list->op.as_list[0], n + SHOW_INCR);
        INDENT(n);
        printf("+\n");
        break;
    }
    }
}

/*           "fo*|bar?"
 *
 *           __ (|) __
 *          /         \
 *        (.)         (.)
 *       /   \       /   \
 *      f    (*)    b    (.)
 *             \        /   \
 *              o      a    (?)
 *                            \
 *                             r
 */

i32 main(void) {
    printf("sizeof(ListTag)  : %zu\n"
           "sizeof(ListOp)   : %zu\n"
           "sizeof(List)     : %zu\n"
           "sizeof(InstTag)  : %zu\n"
           "sizeof(InstOp)   : %zu\n"
           "sizeof(Inst)     : %zu\n"
           "sizeof(Index)    : %zu\n"
           "sizeof(Memory)   : %zu\n\n",
           sizeof(ListTag),
           sizeof(ListOp),
           sizeof(List),
           sizeof(InstTag),
           sizeof(InstOp),
           sizeof(Inst),
           sizeof(Index),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    String  regex = TO_STRING("fo*|bar?");
    List*   expr = parse(memory, regex, 0);
    show_expr(expr, 0);
    printf("\nDone!\n");
    free(memory);
    return EXIT_SUCCESS;
}
