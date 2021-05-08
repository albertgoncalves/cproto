#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://swtch.com/~rsc/regexp/regexp2.html`.
// NOTE: See `https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html#tag_09_04_08`.
// NOTE: See `https://matklad.github.io/2020/04/13/simple-but-powerful-pratt-parsing.html`.

#define CAP_TOKENS    128
#define CAP_EXPRS     128
#define CAP_PRE_INSTS 128
#define CAP_INSTS     64
#define CAP_LABELS    64
#define CAP_THREADS   256
#define CAP_STRING    64
#define CAP_FLAGS     (sizeof(u64) * CAP_INSTS)

typedef uint8_t  u8;
typedef uint64_t u64;
typedef size_t   usize;

typedef int32_t i32;

#define STATIC_ASSERT _Static_assert

typedef enum {
    FALSE = 0,
    TRUE,
} Bool;

typedef struct {
    const char* chars;
    u8          len;
} String;

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "\n%s:%s:%d `%s`\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
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
    u8 index;
    u8 start;
} Thread;

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
    Thread  threads[2][CAP_THREADS];
    u64     flags[2][CAP_INSTS];
} Memory;

typedef struct {
    Thread* buffer;
    u64*    flags;
    u8      len;
} Threads;

typedef struct {
    u8   start;
    u8   end;
    Bool match;
} Bounds;

static void reset(Memory* memory) {
    memory->len_tokens = 0;
    memory->cur_tokens = 0;
    memory->len_exprs = 0;
    memory->len_pre_insts = 0;
    memory->len_labels = 0;
    memory->len_insts = 0;
}

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

static Expr* compile(Memory* memory, String regex) {
    reset(memory);
    set_tokens(memory, regex);
    Expr* expr = parse_expr(memory, BINDING_INIT);
    emit(memory, expr);
    PreInst* pre_inst = alloc_pre_inst(memory);
    pre_inst->tag = PRE_INST_MATCH;
    resolve_labels(memory);
    return expr;
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

static void show_all(Memory* memory, Expr* expr) {
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
    printf("\n");
}

STATIC_ASSERT(CAP_STRING == 64, "CAP_STRING != 64");
static void push_threads(Threads* threads, u8 index, u8 start) {
    if ((threads->flags[index] >> start) & 1lu) {
        return;
    }
    EXIT_IF(CAP_THREADS <= threads->len);
    threads->buffer[threads->len++] = (Thread){
        .index = index,
        .start = start,
    };
    threads->flags[index] |= 1lu << start;
}

static Bounds search(Memory* memory, String string) {
    EXIT_IF(CAP_STRING <= string.len);
    memset(&memory->flags[0][0], 0, 2 * CAP_FLAGS);
    Threads current = {
        .buffer = &memory->threads[0][0],
        .flags = &memory->flags[0][0],
        .len = 0,
    };
    Threads next = {
        .buffer = &memory->threads[1][0],
        .flags = &memory->flags[1][0],
        .len = 0,
    };
    Bounds result = {0};
    for (u8 i = 0; i < string.len; ++i) {
        push_threads(&current, 0, i);
        for (u8 j = 0; j < current.len; ++j) {
            Inst inst = memory->insts[current.buffer[j].index];
            u8   start = current.buffer[j].start;
            switch (inst.tag) {
            case INST_CHAR: {
                if (string.chars[i] == inst.op.as_char) {
                    push_threads(&next, current.buffer[j].index + 1, start);
                }
                break;
            }
            case INST_JUMP: {
                push_threads(&current, inst.op.as_line[0], start);
                break;
            }
            case INST_SPLIT: {
                push_threads(&current, inst.op.as_line[0], start);
                push_threads(&current, inst.op.as_line[1], start);
                break;
            }
            case INST_MATCH: {
                if (!result.match) {
                    result = (Bounds){
                        .start = start,
                        .end = i,
                        .match = TRUE,
                    };
                } else if ((result.start == start) && (result.end < i)) {
                    result.end = i;
                }
                break;
            }
            default: {
                ERROR();
            }
            }
        }
        {
            Thread* buffer = current.buffer;
            u64*    flags = current.flags;
            current = next;
            next = (Threads){
                .buffer = buffer,
                .flags = flags,
                .len = 0,
            };
            memset(&next.flags[0], 0, CAP_FLAGS);
        }
    }
    for (u8 i = 0; i < current.len; ++i) {
        Inst inst = memory->insts[current.buffer[i].index];
        u8   start = current.buffer[i].start;
        switch (inst.tag) {
        case INST_CHAR: {
            break;
        }
        case INST_JUMP: {
            push_threads(&current, inst.op.as_line[0], start);
            break;
        }
        case INST_SPLIT: {
            push_threads(&current, inst.op.as_line[0], start);
            push_threads(&current, inst.op.as_line[1], start);
            break;
        }
        case INST_MATCH: {
            if (!result.match) {
                result = (Bounds){
                    .start = start,
                    .end = string.len,
                    .match = TRUE,
                };
            } else if ((result.start == start) && (result.end < string.len)) {
                result.end = string.len;
            }
            break;
        }
        default: {
            ERROR();
        }
        }
    }
    return result;
}

#define SEARCH(memory, string_literal, start_, end_)               \
    {                                                              \
        Bounds result = search(memory, TO_STRING(string_literal)); \
        EXIT_IF((!result.match) || (result.start != start_) ||     \
                (result.end != end_));                             \
        fprintf(stderr, ".");                                      \
    }

#define NO_SEARCH(memory, string_literal)                         \
    {                                                             \
        EXIT_IF(search(memory, TO_STRING(string_literal)).match); \
        fprintf(stderr, ".");                                     \
    }

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
           "sizeof(Thread)     : %zu\n"
           "sizeof(Threads)    : %zu\n"
           "sizeof(Bounds)     : %zu\n"
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
           sizeof(Thread),
           sizeof(Threads),
           sizeof(Bounds),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    {
        Expr* expr = compile(memory, TO_STRING("fo*|(ba(r|z?))+|jazz"));
        show_all(memory, expr);
        NO_SEARCH(memory, "");
        NO_SEARCH(memory, "???");
        NO_SEARCH(memory, "b");
        NO_SEARCH(memory, "jaz");
        SEARCH(memory, "foooooooooooo", 0, 13);
        SEARCH(memory, "f", 0, 1);
        SEARCH(memory, "ffooo", 0, 1);
        SEARCH(memory, " fooo", 1, 5);
        SEARCH(memory, "ba", 0, 2);
        SEARCH(memory, "bab", 0, 2);
        SEARCH(memory, "baba", 0, 4);
        SEARCH(memory, "bar", 0, 3);
        SEARCH(memory, " baz", 1, 4);
        SEARCH(memory, "  babarbazba", 2, 12);
        SEARCH(memory, "jazz", 0, 4);
        SEARCH(memory, "jazzz", 0, 4);
        fprintf(stderr, "\n");
    }
    {
        compile(memory,
                TO_STRING("a?a?a?a?a?a?a?a?a?a?a?a?a?a?aaaaaaaaaaaaaaaaaaa"));
        NO_SEARCH(memory, "aaaaaaaaaaaaaaaaaa");
        SEARCH(memory, " aaaaaaaaaaaaaaaaaaa", 1, 20);
        SEARCH(memory, " aaaaaaaaaaaaaaaaaaaaaaaaa", 1, 26);
        SEARCH(memory, " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 1, 34);
        SEARCH(memory, " aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa", 1, 34);
        fprintf(stderr, "\n");
    }
    free(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
