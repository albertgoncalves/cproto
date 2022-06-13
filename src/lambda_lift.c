#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define CAP_BUFFER     (1 << 5)
#define CAP_EXPRS      (1 << 6)
#define CAP_EXPR_LISTS (1 << 4)

typedef uint32_t u32;
typedef int32_t  i32;
typedef int64_t  i64;

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
} Str;

#define STR(literal)                \
    ((Str){                         \
        .buffer = literal,          \
        .len = sizeof(literal) - 1, \
    })

typedef struct Expr Expr;
typedef struct List List;

struct List {
    Expr* expr;
    List* next;
    List* last;
};

typedef enum {
    EXPR_ERROR = 0,
    EXPR_I64,
    EXPR_VAR,
    EXPR_STR,
    EXPR_FN0,
    EXPR_FN1,
    EXPR_FN2,
    EXPR_CALL0,
    EXPR_CALL1,
    EXPR_CALL2,
    EXPR_CALL3,
    EXPR_ASSIGN,
    EXPR_UPDATE,
    EXPR_PAIR,
} ExprTag;

typedef struct {
    Str   arg;
    List* exprs;
} ExprFn1;

typedef struct {
    Str   args[2];
    List* exprs;
} ExprFn2;

typedef struct {
    Expr* func;
} ExprCall0;

typedef struct {
    Expr* func;
    Expr* arg;
} ExprCall1;

typedef struct {
    Expr* func;
    Expr* args[2];
} ExprCall2;

typedef struct {
    Expr* func;
    Expr* args[3];
} ExprCall3;

typedef struct {
    Str   var;
    Expr* expr;
} ExprAssign;

typedef struct {
    Str   var;
    Expr* expr;
} ExprUpdate;

typedef union {
    List*      as_fn0;
    ExprFn1    as_fn1;
    ExprFn2    as_fn2;
    Expr*      as_call0;
    ExprCall1  as_call1;
    ExprCall2  as_call2;
    ExprCall3  as_call3;
    ExprAssign as_assign;
    ExprUpdate as_update;
    Expr*      as_pair[2];
    Str        as_str;
    i64        as_i64;
} ExprBody;

struct Expr {
    ExprBody body;
    ExprTag  tag;
};

typedef struct {
    char buffer[CAP_BUFFER];
    u32  len_buffer;
    Expr exprs[CAP_EXPRS];
    u32  len_exprs;
    List lists[CAP_EXPR_LISTS];
    u32  len_lists;
} Memory;

static Expr EXPR_VAR_NEW_SCOPE = {
    .tag = EXPR_VAR,
    .body = {.as_str = STR("@newScope")},
};
static Expr EXPR_VAR_NEW_SCOPE_FROM = {
    .tag = EXPR_VAR,
    .body = {.as_str = STR("@newScopeFrom")},
};
static Expr EXPR_VAR_LOOKUP_SCOPE = {
    .tag = EXPR_VAR,
    .body = {.as_str = STR("@lookupScope")},
};
static Expr EXPR_VAR_INSERT_SCOPE = {
    .tag = EXPR_VAR,
    .body = {.as_str = STR("@insertScope")},
};
static Expr EXPR_VAR_UPDATE_SCOPE = {
    .tag = EXPR_VAR,
    .body = {.as_str = STR("@updateScope")},
};

static u32 COUNT_SCOPES = 0;
static u32 COUNT_FUNCS = 0;

static Memory* alloc_memory(void) {
    void* address = mmap(NULL,
                         sizeof(Memory),
                         PROT_READ | PROT_WRITE,
                         MAP_ANONYMOUS | MAP_PRIVATE,
                         -1,
                         0);
    EXIT_IF(address == MAP_FAILED);
    Memory* memory = (Memory*)address;
    memset(memory, 0, sizeof(Memory));
    return memory;
}

static Expr* alloc_expr(Memory* memory) {
    EXIT_IF(CAP_EXPRS <= memory->len_exprs);
    return &memory->exprs[memory->len_exprs++];
}

static Expr* alloc_i64(Memory* memory, i64 x) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_I64;
    expr->body.as_i64 = x;
    return expr;
}

static Expr* alloc_var(Memory* memory, Str var) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_VAR;
    expr->body.as_str = var;
    return expr;
}

static Expr* alloc_str(Memory* memory, Str str) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_STR;
    expr->body.as_str = str;
    return expr;
}

static Expr* alloc_fn0(Memory* memory, List* exprs) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_FN0;
    expr->body.as_fn0 = exprs;
    return expr;
}

static Expr* alloc_fn1(Memory* memory, Str arg, List* exprs) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_FN1;
    expr->body.as_fn1.arg = arg;
    expr->body.as_fn1.exprs = exprs;
    return expr;
}

static Expr* alloc_call0(Memory* memory, Expr* func) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_CALL0;
    expr->body.as_call0 = func;
    return expr;
}

static Expr* alloc_call1(Memory* memory, Expr* func, Expr* arg) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_CALL1;
    expr->body.as_call1.func = func;
    expr->body.as_call1.arg = arg;
    return expr;
}

static Expr* alloc_call2(Memory* memory, Expr* func, Expr* arg0, Expr* arg1) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_CALL2;
    expr->body.as_call2.func = func;
    expr->body.as_call2.args[0] = arg0;
    expr->body.as_call2.args[1] = arg1;
    return expr;
}

static Expr* alloc_call3(Memory* memory,
                         Expr*   func,
                         Expr*   arg0,
                         Expr*   arg1,
                         Expr*   arg2) {
    Expr* expr = alloc_expr(memory);
    expr->tag = EXPR_CALL3;
    expr->body.as_call3.func = func;
    expr->body.as_call3.args[0] = arg0;
    expr->body.as_call3.args[1] = arg1;
    expr->body.as_call3.args[2] = arg2;
    return expr;
}

static Expr* alloc_assign(Memory* memory, Str var, Expr* expr) {
    Expr* assign = alloc_expr(memory);
    assign->tag = EXPR_ASSIGN;
    assign->body.as_assign.var = var;
    assign->body.as_assign.expr = expr;
    return assign;
}

static Expr* alloc_update(Memory* memory, Str var, Expr* expr) {
    Expr* update = alloc_expr(memory);
    update->tag = EXPR_UPDATE;
    update->body.as_update.var = var;
    update->body.as_update.expr = expr;
    return update;
}

static Expr* alloc_pair(Memory* memory, Expr* expr0, Expr* expr1) {
    Expr* pair = alloc_expr(memory);
    pair->tag = EXPR_PAIR;
    pair->body.as_pair[0] = expr0;
    pair->body.as_pair[1] = expr1;
    return pair;
}

static List* alloc_list(Memory* memory, Expr* expr, List* next) {
    EXIT_IF(!expr);
    EXIT_IF(CAP_EXPR_LISTS <= memory->len_lists);
    List* list = &memory->lists[memory->len_lists++];
    list->expr = expr;
    if (next) {
        list->next = next;
        list->last = next->last;
    } else {
        list->next = NULL;
        list->last = NULL;
    }
    return list;
}

static List* append_list(Memory* memory, List* exprs, Expr* expr) {
    List* last = alloc_list(memory, expr, NULL);
    if (!exprs) {
        return last;
    }
    if (!exprs->last) {
        EXIT_IF(exprs->next);
        exprs->next = last;
        exprs->last = last;
    } else {
        EXIT_IF(!exprs->next);
        exprs->last->next = last;
        exprs->last = last;
    }
    return exprs;
}

static List* merge_lists(List* first, List* second) {
    EXIT_IF(!first);
    if (second) {
        if (!first->last) {
            first->next = second;
        } else {
            first->last->next = second;
        }
        if (!second->last) {
            first->last = second;
        } else {
            first->last = second->last;
        }
    }
    return first;
}

static void print_string(Str str) {
    printf("%.*s", str.len, str.buffer);
}

void print_exprs(List*, char);

static void print_expr(Expr* expr) {
    switch (expr->tag) {
    case EXPR_I64: {
        printf("%ld", expr->body.as_i64);
        break;
    }
    case EXPR_VAR: {
        print_string(expr->body.as_str);
        break;
    }
    case EXPR_STR: {
        putchar('"');
        print_string(expr->body.as_str);
        putchar('"');
        break;
    }
    case EXPR_FN0: {
        printf("(\\ -> ");
        print_exprs(expr->body.as_fn0, ' ');
        putchar(')');
        break;
    }
    case EXPR_FN1: {
        printf("(\\");
        print_string(expr->body.as_fn1.arg);
        printf(" -> ");
        print_exprs(expr->body.as_fn1.exprs, ' ');
        putchar(')');
        break;
    }
    case EXPR_FN2: {
        printf("(\\");
        print_string(expr->body.as_fn2.args[0]);
        putchar(' ');
        print_string(expr->body.as_fn2.args[1]);
        printf(" -> ");
        print_exprs(expr->body.as_fn2.exprs, ' ');
        putchar(')');
        break;
    }
    case EXPR_CALL0: {
        putchar('(');
        print_expr(expr->body.as_call0);
        printf(" ())");
        break;
    }
    case EXPR_CALL1: {
        putchar('(');
        print_expr(expr->body.as_call1.func);
        putchar(' ');
        print_expr(expr->body.as_call1.arg);
        putchar(')');
        break;
    }
    case EXPR_CALL2: {
        putchar('(');
        print_expr(expr->body.as_call2.func);
        putchar(' ');
        print_expr(expr->body.as_call2.args[0]);
        putchar(' ');
        print_expr(expr->body.as_call2.args[1]);
        putchar(')');
        break;
    }
    case EXPR_CALL3: {
        putchar('(');
        print_expr(expr->body.as_call3.func);
        putchar(' ');
        print_expr(expr->body.as_call3.args[0]);
        putchar(' ');
        print_expr(expr->body.as_call3.args[1]);
        putchar(' ');
        print_expr(expr->body.as_call3.args[2]);
        putchar(')');
        break;
    }
    case EXPR_ASSIGN: {
        print_string(expr->body.as_assign.var);
        printf(" := ");
        print_expr(expr->body.as_assign.expr);
        break;
    }
    case EXPR_UPDATE: {
        print_string(expr->body.as_update.var);
        printf(" = ");
        print_expr(expr->body.as_update.expr);
        break;
    }
    case EXPR_PAIR: {
        putchar('(');
        print_expr(expr->body.as_pair[0]);
        printf(", ");
        print_expr(expr->body.as_pair[1]);
        putchar(')');
        break;
    }
    case EXPR_ERROR:
    default: {
        EXIT();
    }
    }
}

void print_exprs(List* exprs, char delim) {
    for (;;) {
        EXIT_IF(!exprs->expr);
        print_expr(exprs->expr);
        if (!exprs->next) {
            return;
        }
        putchar(';');
        putchar(delim);
        exprs = exprs->next;
    }
}

static u32 u32_to_len(u32 x) {
    u32 len = 0;
    do {
        ++len;
        x /= 10;
    } while (x);
    return len;
}

static Str u32_to_string(Memory* memory, u32 x) {
    u32   len = u32_to_len(x);
    char* buffer = &memory->buffer[memory->len_buffer];
    Str   string = {
          .buffer = buffer,
          .len = len,
    };
    EXIT_IF(CAP_BUFFER < (memory->len_buffer + len));
    for (u32 i = 0; i < len; ++i) {
        buffer[(len - i) - 1] = '0' + ((char)(x % 10));
        x /= 10;
    }
    memory->len_buffer += string.len;
    return string;
}

static void push_buffer(Memory* memory, char x) {
    EXIT_IF(CAP_BUFFER <= memory->len_buffer);
    memory->buffer[memory->len_buffer++] = x;
}

static Str get_scope_label(Memory* memory) {
    Str str = {
        .buffer = &memory->buffer[memory->len_buffer],
        .len = 3,
    };
    push_buffer(memory, '_');
    push_buffer(memory, 's');
    str.len += u32_to_string(memory, COUNT_SCOPES++).len;
    push_buffer(memory, '_');
    return str;
}

Expr* inject_scope(Memory*, Expr*, Str);

static void map_inject_scope(Memory* memory, List* exprs, Str scope) {
    for (;;) {
        EXIT_IF(!exprs->expr);
        exprs->expr = inject_scope(memory, exprs->expr, scope);
        if (!exprs->next) {
            return;
        }
        exprs = exprs->next;
    }
}

static List* get_top_scope(Memory* memory, List* exprs) {
    Str scope = get_scope_label(memory);
    map_inject_scope(memory, exprs, scope);
    return alloc_list(
        memory,
        alloc_assign(memory, scope, alloc_call0(memory, &EXPR_VAR_NEW_SCOPE)),
        exprs);
}

static List* get_inner_scope0(Memory* memory, List* exprs, Str parent_scope) {
    Str scope = get_scope_label(memory);
    map_inject_scope(memory, exprs, scope);
    return alloc_list(
        memory,
        alloc_assign(memory,
                     scope,
                     alloc_call1(memory,
                                 &EXPR_VAR_NEW_SCOPE_FROM,
                                 alloc_var(memory, parent_scope))),
        exprs);
}

static List* get_inner_scope1(Memory* memory,
                              List*   exprs,
                              Str     parent_scope,
                              Str     arg) {
    Str scope = get_scope_label(memory);
    map_inject_scope(memory, exprs, scope);
    return alloc_list(
        memory,
        alloc_assign(memory,
                     scope,
                     alloc_call1(memory,
                                 &EXPR_VAR_NEW_SCOPE_FROM,
                                 alloc_var(memory, parent_scope))),
        alloc_list(memory,
                   alloc_call3(memory,
                               &EXPR_VAR_INSERT_SCOPE,
                               alloc_var(memory, scope),
                               alloc_str(memory, arg),
                               alloc_var(memory, arg)),
                   exprs));
}

Expr* inject_scope(Memory* memory, Expr* expr, Str scope) {
    switch (expr->tag) {
    case EXPR_I64:
    case EXPR_STR: {
        return expr;
    }
    case EXPR_VAR: {
        expr->tag = EXPR_STR;
        return alloc_call2(memory,
                           &EXPR_VAR_LOOKUP_SCOPE,
                           alloc_var(memory, scope),
                           expr);
    }
    case EXPR_ASSIGN: {
        Expr* arg1 = alloc_str(memory, expr->body.as_assign.var);
        Expr* arg2 = inject_scope(memory, expr->body.as_assign.expr, scope);
        expr->tag = EXPR_CALL3;
        expr->body.as_call3.func = &EXPR_VAR_INSERT_SCOPE;
        expr->body.as_call3.args[0] = alloc_var(memory, scope);
        expr->body.as_call3.args[1] = arg1;
        expr->body.as_call3.args[2] = arg2;
        return expr;
    }
    case EXPR_UPDATE: {
        Expr* arg1 = alloc_str(memory, expr->body.as_update.var);
        Expr* arg2 = inject_scope(memory, expr->body.as_update.expr, scope);
        expr->tag = EXPR_CALL3;
        expr->body.as_call3.func = &EXPR_VAR_UPDATE_SCOPE;
        expr->body.as_call3.args[0] = alloc_var(memory, scope);
        expr->body.as_call3.args[1] = arg1;
        expr->body.as_call3.args[2] = arg2;
        return expr;
    }
    case EXPR_FN0: {
        expr->tag = EXPR_FN1;
        expr->body.as_fn1.exprs =
            get_inner_scope0(memory, expr->body.as_fn0, scope);
        expr->body.as_fn1.arg = scope;
        return alloc_pair(memory, alloc_var(memory, scope), expr);
    }
    case EXPR_FN1: {
        Str arg = expr->body.as_fn1.arg;
        expr->tag = EXPR_FN2;
        expr->body.as_fn2.exprs =
            get_inner_scope1(memory, expr->body.as_fn1.exprs, scope, arg);
        expr->body.as_fn2.args[0] = scope;
        expr->body.as_fn2.args[1] = arg;
        return alloc_pair(memory, alloc_var(memory, scope), expr);
    }
    case EXPR_CALL0: {
        expr->body.as_call0 = inject_scope(memory, expr->body.as_call0, scope);
        return expr;
    }
    case EXPR_CALL1: {
        expr->body.as_call1.func =
            inject_scope(memory, expr->body.as_call1.func, scope);
        expr->body.as_call1.arg =
            inject_scope(memory, expr->body.as_call1.arg, scope);
        return expr;
    }
    case EXPR_FN2:
    case EXPR_CALL2:
    case EXPR_CALL3:
    case EXPR_PAIR:
    case EXPR_ERROR:
    default: {
        EXIT();
    }
    }
}

static Str get_func_label(Memory* memory) {
    Str str = {
        .buffer = &memory->buffer[memory->len_buffer],
        .len = 3,
    };
    push_buffer(memory, '_');
    push_buffer(memory, 'f');
    str.len += u32_to_string(memory, COUNT_FUNCS++).len;
    push_buffer(memory, '_');
    return str;
}

Expr* extract_func(Memory*, List**, Expr*);

static List* map_extract_func(Memory* memory, List* exprs) {
    List* funcs = NULL;
    for (;;) {
        EXIT_IF(!exprs->expr);
        exprs->expr = extract_func(memory, &funcs, exprs->expr);
        if (!exprs->next) {
            return funcs;
        }
        exprs = exprs->next;
    }
}

Expr* extract_func(Memory* memory, List** funcs, Expr* expr) {
    switch (expr->tag) {
    case EXPR_FN0: {
        Str label = get_func_label(memory);
        *funcs = merge_lists(
            append_list(memory, *funcs, alloc_assign(memory, label, expr)),
            map_extract_func(memory, expr->body.as_fn0));
        return alloc_var(memory, label);
    }
    case EXPR_FN1: {
        Str label = get_func_label(memory);
        *funcs = merge_lists(
            append_list(memory, *funcs, alloc_assign(memory, label, expr)),
            map_extract_func(memory, expr->body.as_fn1.exprs));
        return alloc_var(memory, label);
    }
    case EXPR_FN2: {
        Str label = get_func_label(memory);
        *funcs = merge_lists(
            append_list(memory, *funcs, alloc_assign(memory, label, expr)),
            map_extract_func(memory, expr->body.as_fn2.exprs));
        return alloc_var(memory, label);
    }
    case EXPR_CALL0: {
        expr->body.as_call0 = extract_func(memory, funcs, expr->body.as_call0);
        return expr;
    }
    case EXPR_CALL1: {
        expr->body.as_call1.func =
            extract_func(memory, funcs, expr->body.as_call1.func);
        expr->body.as_call1.arg =
            extract_func(memory, funcs, expr->body.as_call1.arg);
        return expr;
    }
    case EXPR_CALL2: {
        expr->body.as_call2.func =
            extract_func(memory, funcs, expr->body.as_call2.func);
        expr->body.as_call2.args[0] =
            extract_func(memory, funcs, expr->body.as_call2.args[0]);
        expr->body.as_call2.args[1] =
            extract_func(memory, funcs, expr->body.as_call2.args[1]);
        return expr;
    }
    case EXPR_CALL3: {
        expr->body.as_call3.func =
            extract_func(memory, funcs, expr->body.as_call3.func);
        expr->body.as_call3.args[0] =
            extract_func(memory, funcs, expr->body.as_call3.args[0]);
        expr->body.as_call3.args[1] =
            extract_func(memory, funcs, expr->body.as_call3.args[1]);
        expr->body.as_call3.args[2] =
            extract_func(memory, funcs, expr->body.as_call3.args[2]);
        return expr;
    }
    case EXPR_ASSIGN: {
        expr->body.as_assign.expr =
            extract_func(memory, funcs, expr->body.as_assign.expr);
        return expr;
    }
    case EXPR_UPDATE: {
        expr->body.as_update.expr =
            extract_func(memory, funcs, expr->body.as_update.expr);
        return expr;
    }
    case EXPR_PAIR: {
        expr->body.as_pair[0] =
            extract_func(memory, funcs, expr->body.as_pair[0]);
        expr->body.as_pair[1] =
            extract_func(memory, funcs, expr->body.as_pair[1]);
        return expr;
    }
    case EXPR_I64:
    case EXPR_VAR:
    case EXPR_STR: {
        return expr;
    }
    case EXPR_ERROR:
    default: {
        EXIT();
    }
    }
}

i32 main(void) {
    printf("\n"
           "sizeof(Str)     : %zu\n"
           "sizeof(ExprFn2) : %zu\n"
           "sizeof(Expr)    : %zu\n"
           "sizeof(List)    : %zu\n"
           "sizeof(Memory)  : %zu\n"
           "\n",
           sizeof(Str),
           sizeof(ExprFn2),
           sizeof(Expr),
           sizeof(List),
           sizeof(Memory));
    Memory* memory = alloc_memory();
    Expr*   expr0 = alloc_assign(
        memory,
        STR("f"),
        alloc_fn1(memory,
                  STR("x"),
                  alloc_list(memory,
                             alloc_fn0(memory,
                                       alloc_list(memory,
                                                  alloc_var(memory, STR("x")),
                                                  NULL)),
                             NULL)));
    Expr* expr1 = alloc_assign(
        memory,
        STR("g"),
        alloc_fn0(
            memory,
            alloc_list(
                memory,
                alloc_assign(memory, STR("x"), alloc_i64(memory, 0)),
                alloc_list(
                    memory,
                    alloc_update(memory, STR("x"), alloc_i64(memory, -1)),
                    alloc_list(memory, alloc_var(memory, STR("x")), NULL)))));
    Expr* expr2 = alloc_call0(
        memory,
        alloc_call1(memory,
                    alloc_var(memory, STR("f")),
                    alloc_call0(memory, alloc_var(memory, STR("g")))));
    List* exprs =
        alloc_list(memory,
                   expr0,
                   alloc_list(memory, expr1, alloc_list(memory, expr2, NULL)));
    print_exprs(exprs, '\n');
    printf("\n\n");

    exprs = get_top_scope(memory, exprs);
    print_exprs(exprs, '\n');
    printf("\n\n");

    exprs = merge_lists(map_extract_func(memory, exprs), exprs);
    print_exprs(exprs, '\n');
    putchar('\n');

    return OK;
}
