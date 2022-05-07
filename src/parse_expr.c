#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t  i32;
typedef int64_t  i64;

#define OK    0
#define ERROR 1

#define CAP_NODES (1 << 6)

#define EXIT()                                              \
    {                                                       \
        printf("%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(ERROR);                                       \
    }

#define EXIT_IF(condition)                                                   \
    if (condition) {                                                         \
        printf("%s:%s:%d `%s`\n", __FILE__, __func__, __LINE__, #condition); \
        _exit(ERROR);                                                        \
    }

typedef struct {
    const char* buffer;
    u32         len;
} String;

#define STRING(literal)             \
    ((String){                      \
        .buffer = literal,          \
        .len = sizeof(literal) - 1, \
    })

typedef union {
    String as_string;
    i64    as_i64;
} TokenBody;

typedef enum {
    TOKEN_END = 0,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_BACKSLASH,
    TOKEN_ARROW,
    TOKEN_SEMICOLON,
    TOKEN_ASSIGN,
    TOKEN_ADD,
    TOKEN_MUL,
    TOKEN_IDENT,
    TOKEN_I64,
} TokenTag;

typedef struct {
    TokenBody body;
    TokenTag  tag;
} Token;

typedef struct AstExpr AstExpr;

typedef struct {
    String         arg;
    const AstExpr* body;
} AstFn;

typedef union {
    const AstExpr* as_exprs[2];
    AstFn          as_fn;
    String         as_string;
    i64            as_i64;
} AstExprBody;

typedef enum {
    AST_EXPR_CALL,
    AST_EXPR_IDENT,
    AST_EXPR_I64,
    AST_EXPR_FN,
} AstExprTag;

struct AstExpr {
    AstExprBody body;
    AstExprTag  tag;
};


typedef struct {
    AstExpr nodes[CAP_NODES];
    u32     len_nodes;
} Memory;

static Memory* alloc_memory(void) {
    void* address = mmap(NULL,
                         sizeof(Memory),
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE,
                         -1,
                         0);
    EXIT_IF(address == MAP_FAILED);
    Memory* memory = (Memory*)address;
    memory->len_nodes = 0;
    return memory;
}

static AstExpr* alloc_expr(Memory* memory) {
    EXIT_IF(CAP_NODES <= memory->len_nodes);
    return &memory->nodes[memory->len_nodes++];
}

static AstExpr* alloc_expr_ident(Memory* memory, String string) {
    AstExpr* expr = alloc_expr(memory);
    expr->tag = AST_EXPR_IDENT;
    expr->body.as_string = string;
    return expr;
}

static AstExpr* alloc_expr_i64(Memory* memory, i64 x) {
    AstExpr* expr = alloc_expr(memory);
    expr->tag = AST_EXPR_I64;
    expr->body.as_i64 = x;
    return expr;
}

static AstExpr* alloc_expr_call(Memory*        memory,
                                const AstExpr* a,
                                const AstExpr* b) {
    AstExpr* expr = alloc_expr(memory);
    expr->tag = AST_EXPR_CALL;
    expr->body.as_exprs[0] = a;
    expr->body.as_exprs[1] = b;
    return expr;
}

static void print_string(String string) {
    printf("%.*s", string.len, string.buffer);
}

static void print_token(Token token) {
    switch (token.tag) {
    case TOKEN_IDENT: {
        print_string(token.body.as_string);
        break;
    }
    case TOKEN_I64: {
        printf("%ld", token.body.as_i64);
        break;
    }
    case TOKEN_LPAREN: {
        putchar('(');
        break;
    }
    case TOKEN_RPAREN: {
        putchar(')');
        break;
    }
    case TOKEN_BACKSLASH: {
        putchar('\\');
        break;
    }
    case TOKEN_ARROW: {
        printf("->");
        break;
    }
    case TOKEN_SEMICOLON: {
        putchar(';');
        break;
    }
    case TOKEN_ASSIGN: {
        putchar('=');
        break;
    }
    case TOKEN_ADD: {
        putchar('+');
        break;
    }
    case TOKEN_MUL: {
        putchar('*');
        break;
    }
    case TOKEN_END:
    default: {
        EXIT();
    }
    }
}

static void print_tokens(const Token* tokens) {
    for (u32 i = 0;;) {
        print_token(tokens[i++]);
        if (tokens[i].tag == TOKEN_END) {
            putchar('\n');
            return;
        }
        putchar(' ');
    }
}

AstExpr* parse_expr(Memory*, const Token**, u32, u32);

static AstExpr* parse_fn(Memory* memory, const Token** tokens, u32 depth) {
    EXIT_IF((*tokens)->tag != TOKEN_IDENT);
    AstExpr* expr = alloc_expr(memory);
    expr->tag = AST_EXPR_FN;
    expr->body.as_fn.arg = (*tokens)->body.as_string;
    ++(*tokens);
    EXIT_IF((*tokens)->tag != TOKEN_ARROW);
    ++(*tokens);
    expr->body.as_fn.body = parse_expr(memory, tokens, 0, depth);
    return expr;
}

#define PARSE_INFIX(op, binding_left, binding_right)              \
    {                                                             \
        if (binding_left < binding) {                             \
            return expr;                                          \
        }                                                         \
        ++(*tokens);                                              \
        expr = alloc_expr_call(                                   \
            memory,                                               \
            alloc_expr_call(memory,                               \
                            alloc_expr_ident(memory, STRING(op)), \
                            expr),                                \
            parse_expr(memory, tokens, binding_right, depth));    \
    }

AstExpr* parse_expr(Memory*       memory,
                    const Token** tokens,
                    u32           binding,
                    u32           depth) {
    AstExpr* expr;
    switch ((*tokens)->tag) {
    case TOKEN_LPAREN: {
        ++(*tokens);
        expr = parse_expr(memory, tokens, 0, depth + 1);
        EXIT_IF((*tokens)->tag != TOKEN_RPAREN);
        ++(*tokens);
        break;
    }
    case TOKEN_IDENT: {
        expr = alloc_expr_ident(memory, (*tokens)->body.as_string);
        ++(*tokens);
        break;
    }
    case TOKEN_I64: {
        expr = alloc_expr_i64(memory, (*tokens)->body.as_i64);
        ++(*tokens);
        break;
    }
    case TOKEN_BACKSLASH: {
        ++(*tokens);
        expr = parse_fn(memory, tokens, depth);
        break;
    }
    case TOKEN_RPAREN:
    case TOKEN_ARROW:
    case TOKEN_SEMICOLON:
    case TOKEN_ASSIGN:
    case TOKEN_ADD:
    case TOKEN_MUL:
    case TOKEN_END:
    default: {
        EXIT();
    }
    }
    for (;;) {
        switch ((*tokens)->tag) {
        case TOKEN_END: {
            return expr;
        }
        case TOKEN_IDENT:
        case TOKEN_I64: {
#define BINDING_LEFT  9
#define BINDING_RIGHT 10
            if (BINDING_LEFT < binding) {
                return expr;
            }
            expr = alloc_expr_call(
                memory,
                expr,
                parse_expr(memory, tokens, BINDING_RIGHT, depth));
            break;
        }
        case TOKEN_LPAREN: {
            ++(*tokens);
            if (BINDING_LEFT < binding) {
                return expr;
            }
            expr = alloc_expr_call(memory,
                                   expr,
                                   parse_expr(memory, tokens, 0, depth + 1));
            EXIT_IF((*tokens)->tag != TOKEN_RPAREN);
            ++(*tokens);
            break;
        }
        case TOKEN_BACKSLASH: {
            if (BINDING_LEFT < binding) {
                return expr;
            }
            ++(*tokens);
            expr =
                alloc_expr_call(memory, expr, parse_fn(memory, tokens, depth));
            break;
#undef BINDING_LEFT
#undef BINDING_RIGHT
        }
        case TOKEN_ADD: {
            PARSE_INFIX("+", 5, 6);
            break;
        }
        case TOKEN_MUL: {
            PARSE_INFIX("*", 7, 8);
            break;
        }
        case TOKEN_ASSIGN: {
            PARSE_INFIX("=", 4, 3);
            break;
        }
        case TOKEN_SEMICOLON: {
            PARSE_INFIX(";", 1, 2);
            break;
        }
        case TOKEN_RPAREN: {
            EXIT_IF(depth == 0);
            return expr;
        }
        case TOKEN_ARROW:
        default: {
            EXIT();
        }
        }
    }
}

static void print_expr(const AstExpr* expr) {
    switch (expr->tag) {
    case AST_EXPR_IDENT: {
        print_string(expr->body.as_string);
        break;
    }
    case AST_EXPR_I64: {
        printf("%ld", expr->body.as_i64);
        break;
    }
    case AST_EXPR_CALL: {
        print_expr(expr->body.as_exprs[0]);
        putchar('(');
        print_expr(expr->body.as_exprs[1]);
        putchar(')');
        break;
    }
    case AST_EXPR_FN: {
        printf("(\\");
        print_string(expr->body.as_fn.arg);
        printf(" -> ");
        print_expr(expr->body.as_fn.body);
        putchar(')');
        break;
    }
    default: {
        EXIT();
    }
    }
}

static const Token TOKENS[] = {
    {.body = {.as_string = STRING("x")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ASSIGN},
    {.body = {.as_i64 = -123}, .tag = TOKEN_I64},
    {.tag = TOKEN_SEMICOLON},
    {.body = {.as_string = STRING("y")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ASSIGN},
    {.body = {.as_i64 = 45678}, .tag = TOKEN_I64},
    {.tag = TOKEN_SEMICOLON},
    {.body = {.as_string = STRING("f0")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_LPAREN},
    {.body = {.as_string = STRING("f1")}, .tag = TOKEN_IDENT},
    {.body = {.as_string = STRING("x")}, .tag = TOKEN_IDENT},
    {.body = {.as_string = STRING("y")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_RPAREN},
    {.tag = TOKEN_ADD},
    {.tag = TOKEN_LPAREN},
    {.tag = TOKEN_BACKSLASH},
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ARROW},
    {.body = {.as_string = STRING("b")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ASSIGN},
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ADD},
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_MUL},
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_SEMICOLON},
    {.body = {.as_string = STRING("b")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_RPAREN},
    {.body = {.as_string = STRING("z")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_END},
};

i32 main(void) {
    printf("\n"
           "sizeof(String)      : %zu\n"
           "sizeof(TokenBody)   : %zu\n"
           "sizeof(TokenTag)    : %zu\n"
           "sizeof(Token)       : %zu\n"
           "sizeof(AstExprBody) : %zu\n"
           "sizeof(AstExprTag)  : %zu\n"
           "sizeof(AstExpr)     : %zu\n"
           "sizeof(Memory)      : %zu\n"
           "\n",
           sizeof(String),
           sizeof(TokenBody),
           sizeof(TokenTag),
           sizeof(Token),
           sizeof(AstExprBody),
           sizeof(AstExprTag),
           sizeof(AstExpr),
           sizeof(Memory));
    Memory* memory = alloc_memory();
    print_tokens(TOKENS);
    const Token* tokens = TOKENS;
    print_expr(parse_expr(memory, &tokens, 0, 0));
    putchar('\n');
    return OK;
}
