#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://swtch.com/~rsc/regexp/regexp2.html`.
// NOTE: See `https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html#tag_09_04_08`.
// NOTE: See `https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html`.

#define CAP_TOKENS 32
#define CAP_EXPRS  32

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

typedef struct {
    const char* chars;
    u8          len;
} String;

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

#define TO_STRING(literal)          \
    ((String){                      \
        .chars = literal,           \
        .len = sizeof(literal) - 1, \
    })

#define ERROR()                                                      \
    {                                                                \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        exit(EXIT_FAILURE);                                          \
    }

typedef enum {
    TOKEN_CHAR = 0,
    TOKEN_CONCAT,
    TOKEN_OR,
    TOKEN_ZERO_OR_ONE,
    TOKEN_ZERO_OR_MANY,
    TOKEN_ONE_OR_MANY,
} TokenTag;

typedef struct {
    char     char_;
    TokenTag tag;
} Token;

typedef enum {
    EXPR_CHAR = 0,
    EXPR_CONCAT,
    EXPR_OR,
    EXPR_ZERO_OR_ONE,
    EXPR_ZERO_OR_MANY,
    EXPR_ONE_OR_MANY,
} ExprTag;

typedef struct Expr Expr;

typedef union {
    Expr* as_expr[2];
    char  as_char;
} ExprOp;

struct Expr {
    ExprOp  op;
    ExprTag tag;
};

typedef struct {
    Token tokens[CAP_TOKENS];
    u8    len_tokens;
    u8    cur_tokens;
    Expr  exprs[CAP_EXPRS];
    u8    len_exprs;
} Memory;

/*
#define CAP_LABELS 32
#define CAP_INSTS  32

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
    u8    labels[CAP_LABELS];
    Inst  insts[CAP_INSTS];
    Index index;
} Memory;
*/

static Token* alloc_token(Memory* memory) {
    EXIT_IF(CAP_TOKENS <= memory->len_tokens);
    return &memory->tokens[memory->len_tokens++];
}

static Expr* alloc_expr(Memory* memory) {
    EXIT_IF(CAP_EXPRS <= memory->len_exprs);
    return &memory->exprs[memory->len_exprs++];
}

static void set_tokens(Memory* memory, String string) {
    for (u8 i = 0; i < string.len; ++i) {
        switch (string.chars[i]) {
        case '|': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_OR;
            break;
        }
        case '?': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_ZERO_OR_ONE;
            break;
        }
        case '*': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_ZERO_OR_MANY;
            break;
        }
        case '+': {
            Token* token = alloc_token(memory);
            token->tag = TOKEN_ONE_OR_MANY;
            break;
        }
        default: {
            if ((memory->len_tokens != 0) &&
                (memory->tokens[memory->len_tokens - 1].tag == TOKEN_CHAR))
            {
                Token* token = alloc_token(memory);
                token->tag = TOKEN_CONCAT;
            }
            Token* token = alloc_token(memory);
            token->char_ = string.chars[i];
            token->tag = TOKEN_CHAR;
        }
        }
    }
}

static void show_token(Token token) {
    switch (token.tag) {
    case TOKEN_CHAR: {
        printf("'%c'\n", token.char_);
        break;
    }
    case TOKEN_CONCAT: {
        printf(" .\n");
        break;
    }
    case TOKEN_OR: {
        printf(" |\n");
        break;
    }
    case TOKEN_ZERO_OR_ONE: {
        printf(" ?\n");
        break;
    }
    case TOKEN_ZERO_OR_MANY: {
        printf(" *\n");
        break;
    }
    case TOKEN_ONE_OR_MANY: {
        printf(" +\n");
        break;
    }
    }
}

#define TOKENS_EMPTY(memory) (memory->len_tokens <= memory->cur_tokens)

static Token pop_token(Memory* memory) {
    EXIT_IF(TOKENS_EMPTY(memory));
    return memory->tokens[memory->cur_tokens++];
}

#define SET_INFIX(memory, prev_binding, tag_, binding, expr_) \
    {                                                         \
        Expr* infix = alloc_expr(memory);                     \
        infix->tag = tag_;                                    \
        if (binding < prev_binding) {                         \
            return expr_;                                     \
        }                                                     \
        pop_token(memory);                                    \
        infix->op.as_expr[0] = expr_;                         \
        infix->op.as_expr[1] = parse_expr(memory, binding);   \
        expr_ = infix;                                        \
    }

#define SET_POSTFIX(memory, prev_binding, tag_, binding, expr_) \
    {                                                           \
        Expr* postfix = alloc_expr(memory);                     \
        postfix->tag = tag_;                                    \
        if (binding < prev_binding) {                           \
            return expr_;                                       \
        }                                                       \
        pop_token(memory);                                      \
        postfix->op.as_expr[0] = expr_;                         \
        expr_ = postfix;                                        \
    }

static Expr* parse_expr(Memory* memory, u8 prev_binding) {
    if (TOKENS_EMPTY(memory)) {
        return NULL;
    }
    Expr* expr = alloc_expr(memory);
    {
        Token token = pop_token(memory);
        switch (token.tag) {
        case TOKEN_CHAR: {
            expr->tag = EXPR_CHAR;
            expr->op.as_char = token.char_;
            break;
        }
        case TOKEN_CONCAT:
        case TOKEN_OR:
        case TOKEN_ZERO_OR_ONE:
        case TOKEN_ZERO_OR_MANY:
        case TOKEN_ONE_OR_MANY: {
            ERROR();
        }
        }
    }
    while (!TOKENS_EMPTY(memory)) {
        Token token = memory->tokens[memory->cur_tokens];
        switch (token.tag) {
        case TOKEN_CONCAT: {
            SET_INFIX(memory, prev_binding, EXPR_CONCAT, 2, expr);
            break;
        }
        case TOKEN_OR: {
            SET_INFIX(memory, prev_binding, EXPR_OR, 1, expr);
            break;
        }
        case TOKEN_ZERO_OR_ONE: {
            SET_POSTFIX(memory, prev_binding, EXPR_ZERO_OR_ONE, 3, expr);
            break;
        }
        case TOKEN_ZERO_OR_MANY: {
            SET_POSTFIX(memory, prev_binding, EXPR_ZERO_OR_MANY, 3, expr);
            break;
        }
        case TOKEN_ONE_OR_MANY: {
            SET_POSTFIX(memory, prev_binding, EXPR_ONE_OR_MANY, 3, expr);
            break;
        }
        case TOKEN_CHAR: {
            ERROR();
        }
        }
    }
    return expr;
}

#define INDENT(n)                    \
    {                                \
        for (u8 i = 0; i < n; ++i) { \
            printf(" ");             \
        }                            \
    }

#define PAD 2

#define SHOW_ONE(expr, string_literal, n)        \
    {                                            \
        show_expr(expr->op.as_expr[0], n + PAD); \
        INDENT(n);                               \
        printf(string_literal "\n");             \
    }

#define SHOW_BOTH(expr, string_literal, n)       \
    {                                            \
        show_expr(expr->op.as_expr[1], n + PAD); \
        INDENT(n);                               \
        printf(string_literal "\n");             \
        show_expr(expr->op.as_expr[0], n + PAD); \
    }

void show_expr(Expr*, u8);
void show_expr(Expr* expr, u8 n) {
    if (!expr) {
        return;
    }
    switch (expr->tag) {
    case EXPR_CHAR: {
        INDENT(n);
        printf("'%c'\n", expr->op.as_char);
        break;
    }
    case EXPR_CONCAT: {
        SHOW_BOTH(expr, ".", n);
        break;
    }
    case EXPR_OR: {
        SHOW_BOTH(expr, "|", n);
        break;
    }
    case EXPR_ZERO_OR_ONE: {
        SHOW_ONE(expr, "?", n);
        break;
    }
    case EXPR_ZERO_OR_MANY: {
        SHOW_ONE(expr, "*", n);
        break;
    }
    case EXPR_ONE_OR_MANY: {
        SHOW_ONE(expr, "+", n);
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
 *              \       /   \
 *               o     a    (?)
 *                             \
 *                              r
 */

i32 main(void) {
    printf("sizeof(TokenTag) : %zu\n"
           "sizeof(Token)    : %zu\n"
           "sizeof(ExprTag)  : %zu\n"
           "sizeof(ExprOp)   : %zu\n"
           "sizeof(Expr)     : %zu\n"
           "sizeof(Memory)   : %zu\n\n",
           sizeof(TokenTag),
           sizeof(Token),
           sizeof(ExprTag),
           sizeof(ExprOp),
           sizeof(Expr),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    String  regex = TO_STRING("fo*|bar?|j+");
    set_tokens(memory, regex);
    for (u8 i = 0; i < memory->len_tokens; ++i) {
        show_token(memory->tokens[i]);
    }
    show_expr(parse_expr(memory, 0), 0);
    printf("\nDone!\n");
    free(memory);
    return EXIT_SUCCESS;
}
