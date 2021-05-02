#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://swtch.com/~rsc/regexp/regexp2.html`.
// NOTE: See `https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html#tag_09_04_08`.
// NOTE: See `https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html`.

#define CAP_TOKENS    64
#define CAP_EXPRS     64
#define CAP_PRE_INSTS 64
#define CAP_INSTS     64
#define CAP_LABELS    64

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

#define STATIC_ASSERT _Static_assert

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

typedef struct {
    u8 open_;
    u8 close_;
} Parens;

typedef enum {
    TOKEN_CHAR = 0,
    TOKEN_CONCAT,
    TOKEN_OR,
    TOKEN_ZERO_OR_ONE,
    TOKEN_ZERO_OR_MANY,
    TOKEN_ONE_OR_MANY,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    COUNT_TOKEN_TAG,
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

typedef enum {
    PRE_INST_LABEL = 0,
    PRE_INST_MATCH,
    PRE_INST_CHAR,
    PRE_INST_JUMP,
    PRE_INST_SPLIT,
    COUNT_PRE_INST_TAG,
} PreInstTag;

typedef union {
    u8   as_label[2];
    u8   as_line[2];
    char as_char;
} PreInstOp;

typedef struct {
    PreInstOp  op;
    PreInstTag tag;
} PreInst;

typedef enum {
    INST_MATCH = 0,
    INST_CHAR,
    INST_JUMP,
    INST_SPLIT,
} InstTag;

typedef union {
    u8   as_line[2];
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
    Token   tokens[CAP_TOKENS];
    u8      len_tokens;
    u8      cur_tokens;
    Expr    exprs[CAP_EXPRS];
    u8      len_exprs;
    PreInst pre_insts[CAP_PRE_INSTS];
    u8      len_pre_insts;
    u8      labels[CAP_LABELS];
    u8      len_labels;
    Inst    insts[CAP_INSTS];
    u8      len_insts;
} Memory;

static Token* alloc_token(Memory* memory) {
    EXIT_IF(CAP_TOKENS <= memory->len_tokens);
    return &memory->tokens[memory->len_tokens++];
}

static Expr* alloc_expr(Memory* memory) {
    EXIT_IF(CAP_EXPRS <= memory->len_exprs);
    return &memory->exprs[memory->len_exprs++];
}

static PreInst* alloc_pre_inst(Memory* memory) {
    EXIT_IF(CAP_PRE_INSTS <= memory->len_pre_insts);
    return &memory->pre_insts[memory->len_pre_insts++];
}

static Inst* alloc_inst(Memory* memory) {
    EXIT_IF(CAP_INSTS <= memory->len_insts);
    return &memory->insts[memory->len_insts++];
}

#define SET_CONCAT(memory)                                              \
    if ((memory->len_tokens != 0) &&                                    \
        (memory->tokens[memory->len_tokens - 1].tag != TOKEN_OR) &&     \
        (memory->tokens[memory->len_tokens - 1].tag != TOKEN_CONCAT) && \
        (memory->tokens[memory->len_tokens - 1].tag != TOKEN_LPAREN))   \
    {                                                                   \
        Token* token = alloc_token(memory);                             \
        token->tag = TOKEN_CONCAT;                                      \
    }

STATIC_ASSERT(COUNT_TOKEN_TAG == 8, "COUNT_TOKEN_TAG != 8");
static void set_tokens(Memory* memory, String string) {
    Parens parens = {0};
    for (u8 i = 0; i < string.len; ++i) {
        EXIT_IF(parens.open_ < parens.close_);
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
        case '(': {
            ++parens.open_;
            SET_CONCAT(memory);
            Token* token = alloc_token(memory);
            token->tag = TOKEN_LPAREN;
            break;
        }
        case ')': {
            ++parens.close_;
            Token* token = alloc_token(memory);
            token->tag = TOKEN_RPAREN;
            break;
        }
        default: {
            SET_CONCAT(memory);
            Token* token = alloc_token(memory);
            token->char_ = string.chars[i];
            token->tag = TOKEN_CHAR;
        }
        }
    }
    EXIT_IF(parens.open_ != parens.close_);
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
    case TOKEN_LPAREN: {
        printf(" (\n");
        break;
    }
    case TOKEN_RPAREN: {
        printf(" )\n");
        break;
    }
    case COUNT_TOKEN_TAG:
    default: {
        ERROR();
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

#define BINDING_INIT    0
#define BINDING_PAREN   BINDING_INIT
#define BINDING_OR      1
#define BINDING_CONCAT  2
#define BINDING_POSTFIX 3

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
        case TOKEN_LPAREN: {
            expr = parse_expr(memory, BINDING_PAREN);
            break;
        }
        case TOKEN_CONCAT:
        case TOKEN_OR:
        case TOKEN_ZERO_OR_ONE:
        case TOKEN_ZERO_OR_MANY:
        case TOKEN_ONE_OR_MANY:
        case TOKEN_RPAREN:
        case COUNT_TOKEN_TAG:
        default: {
            ERROR();
        }
        }
    }
    while (!TOKENS_EMPTY(memory)) {
        Token token = memory->tokens[memory->cur_tokens];
        switch (token.tag) {
        case TOKEN_CONCAT: {
            SET_INFIX(memory, prev_binding, EXPR_CONCAT, BINDING_CONCAT, expr);
            break;
        }
        case TOKEN_OR: {
            SET_INFIX(memory, prev_binding, EXPR_OR, BINDING_OR, expr);
            break;
        }
        case TOKEN_ZERO_OR_ONE: {
            SET_POSTFIX(memory,
                        prev_binding,
                        EXPR_ZERO_OR_ONE,
                        BINDING_POSTFIX,
                        expr);
            break;
        }
        case TOKEN_ZERO_OR_MANY: {
            SET_POSTFIX(memory,
                        prev_binding,
                        EXPR_ZERO_OR_MANY,
                        BINDING_POSTFIX,
                        expr);
            break;
        }
        case TOKEN_ONE_OR_MANY: {
            SET_POSTFIX(memory,
                        prev_binding,
                        EXPR_ONE_OR_MANY,
                        BINDING_POSTFIX,
                        expr);
            break;
        }
        case TOKEN_LPAREN: {
            pop_token(memory);
            expr = parse_expr(memory, BINDING_PAREN);
            break;
        }
        case TOKEN_RPAREN: {
            if (BINDING_PAREN < prev_binding) {
                return expr;
            }
            pop_token(memory);
            return expr;
        }
        case TOKEN_CHAR:
        case COUNT_TOKEN_TAG:
        default: {
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
        SHOW_BOTH(expr, " .", n);
        break;
    }
    case EXPR_OR: {
        SHOW_BOTH(expr, " |", n);
        break;
    }
    case EXPR_ZERO_OR_ONE: {
        SHOW_ONE(expr, " ?", n);
        break;
    }
    case EXPR_ZERO_OR_MANY: {
        SHOW_ONE(expr, " *", n);
        break;
    }
    case EXPR_ONE_OR_MANY: {
        SHOW_ONE(expr, " +", n);
        break;
    }
    default: {
        ERROR();
    }
    }
}

#define EMIT_PRE_INST_LABEL(tag_, label_)           \
    {                                               \
        PreInst* pre_inst = alloc_pre_inst(memory); \
        pre_inst->tag = tag_;                       \
        pre_inst->op.as_label[0] = label_;          \
    }

#define EMIT_SPLIT(label_0, label_1)                \
    {                                               \
        PreInst* pre_inst = alloc_pre_inst(memory); \
        pre_inst->tag = PRE_INST_SPLIT;             \
        pre_inst->op.as_label[0] = label_0;         \
        pre_inst->op.as_label[1] = label_1;         \
    }

void emit(Memory*, Expr*);
void emit(Memory* memory, Expr* expr) {
    if (!expr) {
        return;
    }
    switch (expr->tag) {
    case EXPR_CHAR: {
        PreInst* pre_inst = alloc_pre_inst(memory);
        pre_inst->tag = PRE_INST_CHAR;
        pre_inst->op.as_char = expr->op.as_char;
        break;
    }
    case EXPR_CONCAT: {
        emit(memory, expr->op.as_expr[0]);
        emit(memory, expr->op.as_expr[1]);
        break;
    }
    case EXPR_OR: {
        u8 label_0 = memory->len_labels++;
        u8 label_1 = memory->len_labels++;
        u8 label_2 = memory->len_labels++;
        EMIT_SPLIT(label_0, label_1);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_0);
        emit(memory, expr->op.as_expr[0]);
        EMIT_PRE_INST_LABEL(PRE_INST_JUMP, label_2);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_1);
        emit(memory, expr->op.as_expr[1]);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_2);
        break;
    }
    case EXPR_ZERO_OR_ONE: {
        u8 label_0 = memory->len_labels++;
        u8 label_1 = memory->len_labels++;
        EMIT_SPLIT(label_0, label_1);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_0);
        emit(memory, expr->op.as_expr[0]);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_1);
        break;
    }
    case EXPR_ZERO_OR_MANY: {
        u8 label_0 = memory->len_labels++;
        u8 label_1 = memory->len_labels++;
        u8 label_2 = memory->len_labels++;
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_0);
        EMIT_SPLIT(label_1, label_2);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_1);
        emit(memory, expr->op.as_expr[0]);
        EMIT_PRE_INST_LABEL(PRE_INST_JUMP, label_0);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_2);
        break;
    }
    case EXPR_ONE_OR_MANY: {
        u8 label_0 = memory->len_labels++;
        u8 label_1 = memory->len_labels++;
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_0);
        emit(memory, expr->op.as_expr[0]);
        EMIT_SPLIT(label_0, label_1);
        EMIT_PRE_INST_LABEL(PRE_INST_LABEL, label_1);
        break;
    }
    default: {
        ERROR();
    }
    }
}

static void resolve_labels(Memory* memory) {
    EXIT_IF(CAP_LABELS <= memory->len_labels);
    u8 line = 0;
    for (u8 i = 0; i < memory->len_pre_insts; ++i) {
        switch (memory->pre_insts[i].tag) {
        case PRE_INST_LABEL: {
            memory->labels[memory->pre_insts[i].op.as_label[0]] = line;
            break;
        }
        case PRE_INST_MATCH:
        case PRE_INST_CHAR:
        case PRE_INST_JUMP:
        case PRE_INST_SPLIT: {
            ++line;
            break;
        }
        case COUNT_PRE_INST_TAG:
        default: {
            ERROR();
        }
        }
    }
    for (u8 i = 0; i < memory->len_pre_insts; ++i) {
        switch (memory->pre_insts[i].tag) {
        case PRE_INST_LABEL: {
            break;
        }
        case PRE_INST_JUMP: {
            Inst* inst = alloc_inst(memory);
            inst->tag = INST_JUMP;
            inst->op.as_line[0] =
                memory->labels[memory->pre_insts[i].op.as_label[0]];
            EXIT_IF(line <= inst->op.as_line[0]);
            break;
        }
        case PRE_INST_SPLIT: {
            Inst* inst = alloc_inst(memory);
            inst->tag = INST_SPLIT;
            inst->op.as_line[0] =
                memory->labels[memory->pre_insts[i].op.as_label[0]];
            EXIT_IF(line <= inst->op.as_line[0]);
            inst->op.as_line[1] =
                memory->labels[memory->pre_insts[i].op.as_label[1]];
            EXIT_IF(line <= inst->op.as_line[1]);
            break;
        }
        case PRE_INST_MATCH: {
            Inst* inst = alloc_inst(memory);
            inst->tag = INST_MATCH;
            break;
        }
        case PRE_INST_CHAR: {
            Inst* inst = alloc_inst(memory);
            inst->tag = INST_CHAR;
            inst->op.as_char = memory->pre_insts[i].op.as_char;
            break;
        }
        case COUNT_PRE_INST_TAG:
        default: {
            ERROR();
        }
        }
    }
}

#define LINE_FMT "%hhu"

static void show_inst(Inst inst) {
    switch (inst.tag) {
    case INST_MATCH: {
        printf("\tmatch\n");
        break;
    }
    case INST_CHAR: {
        printf("\tchar\t'%c'\n", inst.op.as_char);
        break;
    }
    case INST_JUMP: {
        printf("\tjump\t" LINE_FMT "\n", inst.op.as_line[0]);
        break;
    }
    case INST_SPLIT:
        printf("\tsplit\t" LINE_FMT ",\t" LINE_FMT "\n",
               inst.op.as_line[0],
               inst.op.as_line[1]);
        break;
    default: {
        ERROR();
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
    printf("\n"
           "sizeof(TokenTag)   : %zu\n"
           "sizeof(Token)      : %zu\n"
           "sizeof(ExprTag)    : %zu\n"
           "sizeof(ExprOp)     : %zu\n"
           "sizeof(Expr)       : %zu\n"
           "sizeof(PreInstTag) : %zu\n"
           "sizeof(PreInstOp)  : %zu\n"
           "sizeof(PreInst)    : %zu\n"
           "sizeof(InstTag)    : %zu\n"
           "sizeof(InstOp)     : %zu\n"
           "sizeof(Inst)       : %zu\n"
           "sizeof(Index)      : %zu\n"
           "sizeof(Memory)     : %zu\n"
           "\n",
           sizeof(TokenTag),
           sizeof(Token),
           sizeof(ExprTag),
           sizeof(ExprOp),
           sizeof(Expr),
           sizeof(PreInstTag),
           sizeof(PreInstOp),
           sizeof(PreInst),
           sizeof(InstTag),
           sizeof(InstOp),
           sizeof(Inst),
           sizeof(Index),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    Expr*   expr;
    {
        String regex = TO_STRING("fo*|(ba(r|z?))+|jazz");
        set_tokens(memory, regex);
        expr = parse_expr(memory, BINDING_INIT);
        {
            memory->len_pre_insts = 0;
            memory->len_labels = 0;
            emit(memory, expr);
            PreInst* pre_inst = alloc_pre_inst(memory);
            pre_inst->tag = PRE_INST_MATCH;
        }
        resolve_labels(memory);
    }
    {
        for (u8 i = 0; i < memory->len_tokens; ++i) {
            show_token(memory->tokens[i]);
        }
        printf("\n");
        show_expr(expr, 0);
        printf("\n");
        for (u8 i = 0; i < memory->len_insts; ++i) {
            printf("%3hhu", i);
            show_inst(memory->insts[i]);
        }
    }
    free(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
