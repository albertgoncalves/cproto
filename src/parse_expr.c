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
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_BACKSLASH,
    TOKEN_ARROW,
    TOKEN_ADD,
    TOKEN_IDENT,
} TokenTag;

typedef struct {
    TokenBody body;
    TokenTag  tag;
} Token;

typedef struct AstExpr AstExpr;

typedef struct {
    String   arg;
    AstExpr* body;
} AstFn;

typedef union {
    AstExpr* as_exprs[2];
    AstFn    as_fn;
    String   as_string;
} AstExprBody;

typedef enum {
    AST_EXPR_CALL,
    AST_EXPR_IDENT,
    AST_EXPR_FN,
} AstExprTag;

struct AstExpr {
    AstExprBody body;
    AstExprTag  tag;
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

AstExpr* parse_expr(Token**, u32, u32);

static AstExpr* parse_fn(Token** tokens, u32 depth) {
    EXIT_IF((*tokens)->tag != TOKEN_IDENT);
    AstExpr* expr = alloc_expr();
    expr->tag = AST_EXPR_FN;
    expr->body.as_fn.arg = (*tokens)->body.as_string;
    ++(*tokens);
    EXIT_IF((*tokens)->tag != TOKEN_ARROW);
    ++(*tokens);
    expr->body.as_fn.body = parse_expr(tokens, 0, depth);
    return expr;
}

AstExpr* parse_expr(Token** tokens, u32 binding, u32 depth) {
    AstExpr* left;
    switch ((*tokens)->tag) {
    case TOKEN_LPAREN: {
        ++(*tokens);
        left = parse_expr(tokens, 0, depth + 1);
        EXIT_IF((*tokens)->tag != TOKEN_RPAREN);
        ++(*tokens);
        break;
    }
    case TOKEN_IDENT: {
        left = alloc_expr();
        left->tag = AST_EXPR_IDENT;
        left->body.as_string = (*tokens)->body.as_string;
        ++(*tokens);
        break;
    }
    case TOKEN_BACKSLASH: {
        ++(*tokens);
        left = parse_fn(tokens, depth);
        break;
    }
    case TOKEN_END:
    case TOKEN_RPAREN:
    case TOKEN_ARROW:
    case TOKEN_ADD:
    default: {
        EXIT();
    }
    }
    for (;;) {
        switch ((*tokens)->tag) {
        case TOKEN_END: {
            return left;
        }
        case TOKEN_ADD: {
#define BINDING_LEFT  1
#define BINDING_RIGHT 2
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
            call1->body.as_exprs[1] = parse_expr(tokens, BINDING_RIGHT, depth);
#undef BINDING_LEFT
#undef BINDING_RIGHT
            left = call1;
            break;
        }
        case TOKEN_IDENT: {
#define BINDING_LEFT  3
#define BINDING_RIGHT 4
            if (BINDING_LEFT < binding) {
                return left;
            }
            AstExpr* call = alloc_expr();
            call->tag = AST_EXPR_CALL;
            call->body.as_exprs[0] = left;
            call->body.as_exprs[1] = parse_expr(tokens, BINDING_RIGHT, depth);
            left = call;
            break;
        }
        case TOKEN_LPAREN: {
            ++(*tokens);
            if (BINDING_LEFT < binding) {
                return left;
            }
            AstExpr* call = alloc_expr();
            call->tag = AST_EXPR_CALL;
            call->body.as_exprs[0] = left;
            call->body.as_exprs[1] = parse_expr(tokens, 0, depth + 1);
            EXIT_IF((*tokens)->tag != TOKEN_RPAREN);
            ++(*tokens);
            left = call;
            break;
        }
        case TOKEN_BACKSLASH: {
            if (BINDING_LEFT < binding) {
                return left;
            }
            ++(*tokens);
            AstExpr* call = alloc_expr();
            call->tag = AST_EXPR_CALL;
            call->body.as_exprs[0] = left;
            call->body.as_exprs[1] = parse_fn(tokens, depth);
#undef BINDING_LEFT
#undef BINDING_RIGHT
            left = call;
            break;
        }
        case TOKEN_RPAREN: {
            EXIT_IF(depth == 0);
            return left;
        }
        case TOKEN_ARROW:
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

static Token TOKENS[] = {
    // f0 (f1 x y) + f0 z
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
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
    {.tag = TOKEN_ADD},
    {.body = {.as_string = STRING("a")}, .tag = TOKEN_IDENT},
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
    print_expr(parse_expr(&tokens, 0, 0));
    putchar('\n');
    return OK;
}
