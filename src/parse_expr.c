#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t  i32;

#define OK    0
#define ERROR 1

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
} TokenBody;

typedef enum {
    TOKEN_END = 0,
    TOKEN_ADD,
    TOKEN_IDENT,
} TokenTag;

typedef struct {
    TokenBody body;
    TokenTag  tag;
} Token;

typedef struct AstExpr AstExpr;

typedef union {
    AstExpr* as_exprs[2];
    String   as_string;
} AstExprBody;

typedef enum {
    AST_EXPR_CALL,
    AST_EXPR_IDENT,
} AstExprTag;

struct AstExpr {
    AstExprBody body;
    AstExprTag  tag;
};

static Token TOKENS[] = {
    // f0 x y + f1 z
    {.body = {.as_string = STRING("f0")}, .tag = TOKEN_IDENT},
    {.body = {.as_string = STRING("x")}, .tag = TOKEN_IDENT},
    {.body = {.as_string = STRING("y")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ADD},
    {.body = {.as_string = STRING("f1")}, .tag = TOKEN_IDENT},
    {.body = {.as_string = STRING("z")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_END},
};

#define CAP_NODES (1 << 5)

static AstExpr NODES[CAP_NODES];
static u32     LEN_NODES = 0;

static AstExpr* alloc_expr(void) {
    EXIT_IF(CAP_NODES <= LEN_NODES);
    return &NODES[LEN_NODES++];
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
    case TOKEN_ADD: {
        putchar('+');
        break;
    }
    case TOKEN_END:
    default: {
        EXIT();
    }
    }
}

static void print_tokens(Token* tokens) {
    for (u32 i = 0;;) {
        print_token(tokens[i++]);
        if (tokens[i].tag == TOKEN_END) {
            putchar('\n');
            return;
        }
        putchar(' ');
    }
}

static AstExpr* parse_expr(Token** tokens, u32 binding) {
    EXIT_IF((*tokens)->tag != TOKEN_IDENT);
    AstExpr* left = alloc_expr();
    left->tag = AST_EXPR_IDENT;
    left->body.as_string = (*tokens)->body.as_string;
    ++(*tokens);
    for (;;) {
        switch ((*tokens)->tag) {
        case TOKEN_END: {
            return left;
        }
        case TOKEN_ADD: {
#define BINDING_LEFT  1
#define BENDING_RIGHT 2
            if (BINDING_LEFT < binding) {
                return left;
            }
            ++(*tokens);

            AstExpr* call0 = alloc_expr();
            call0->tag = AST_EXPR_CALL;
            call0->body.as_exprs[0] = alloc_expr();
            call0->body.as_exprs[0]->tag = AST_EXPR_IDENT;
            call0->body.as_exprs[0]->body.as_string = STRING("+");
            call0->body.as_exprs[1] = left;

            AstExpr* call1 = alloc_expr();
            call1->tag = AST_EXPR_CALL;
            call1->body.as_exprs[0] = call0;
            call1->body.as_exprs[1] = parse_expr(tokens, BENDING_RIGHT);
            ;
#undef BINDING_LEFT
#undef BINDIND_RIGHT
            left = call1;
            break;
        }
        case TOKEN_IDENT: {
#define BINDING_LEFT  3
#define BENDING_RIGHT 4
            if (BINDING_LEFT < binding) {
                return left;
            }
            AstExpr* call = alloc_expr();
            call->tag = AST_EXPR_CALL;
            call->body.as_exprs[0] = left;
            call->body.as_exprs[1] = parse_expr(tokens, BENDING_RIGHT);
#undef BINDING_LEFT
#undef BINDIND_RIGHT
            left = call;
            break;
        }
        default: {
            EXIT();
        }
        }
    }
}

static void print_expr(AstExpr* expr) {
    switch (expr->tag) {
    case AST_EXPR_IDENT: {
        print_string(expr->body.as_string);
        break;
    }
    case AST_EXPR_CALL: {
        putchar('(');
        print_expr(expr->body.as_exprs[0]);
        putchar(' ');
        print_expr(expr->body.as_exprs[1]);
        putchar(')');
        break;
    }
    default: {
        EXIT();
    }
    }
}

i32 main(void) {
    printf("\n"
           "sizeof(String)      : %zu\n"
           "sizeof(TokenBody)   : %zu\n"
           "sizeof(TokenTag)    : %zu\n"
           "sizeof(Token)       : %zu\n"
           "sizeof(AstExprBody) : %zu\n"
           "sizeof(AstExprTag)  : %zu\n"
           "sizeof(AstExpr)     : %zu\n"
           "\n",
           sizeof(String),
           sizeof(TokenBody),
           sizeof(TokenTag),
           sizeof(Token),
           sizeof(AstExprBody),
           sizeof(AstExprTag),
           sizeof(AstExpr));
    print_tokens(TOKENS);
    Token* tokens = TOKENS;
    print_expr(parse_expr(&tokens, 0));
    putchar('\n');
    return OK;
}