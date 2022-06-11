#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define CAP_LISTS  (1 << 5)
#define CAP_SCOPES (1 << 5)

typedef int32_t i32;
typedef int64_t i64;

typedef uint32_t u32;
typedef uint64_t u64;

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

typedef struct List List;

struct List {
    char* key_chars;
    u64   key_len;
    i64   value;
    List* next;
};

typedef struct Scope Scope;

struct Scope {
    List*  list;
    Scope* parent;
};

typedef struct {
    List  lists[CAP_LISTS];
    u64   len_lists;
    Scope scopes[CAP_SCOPES];
    u64   len_scopes;
} Memory;

static Memory MEMORY = {0};

void   memory_init(void);
Scope* scope_new(void);
Scope* scope_new_from(Scope*);
i64    scope_lookup(Scope*, char*, u64);
void   scope_insert(Scope*, char*, u64, i64);
void   scope_update(Scope*, char*, u64, i64);

void memory_init(void) {
    memset(&MEMORY, 0, sizeof(Memory));
}

Scope* scope_new(void) {
    return scope_new_from(NULL);
}

Scope* scope_new_from(Scope* parent) {
    EXIT_IF(CAP_SCOPES <= MEMORY.len_scopes);
    Scope* scope = &MEMORY.scopes[MEMORY.len_scopes++];
    scope->parent = parent;
    return scope;
}

#define EQ(a_chars, a_len, b_chars, b_len) \
    ((a_len == b_len) && (!memcmp(a_chars, b_chars, a_len)))

i64 scope_lookup(Scope* scope, char* key_chars, u64 key_len) {
    while (scope) {
        List* list = scope->list;
        while (list) {
            if (EQ(key_chars, key_len, list->key_chars, list->key_len)) {
                return list->value;
            }
            list = list->next;
        }
        scope = scope->parent;
    }
    EXIT();
}

void scope_insert(Scope* scope, char* key_chars, u64 key_len, i64 value) {
    EXIT_IF(CAP_LISTS <= MEMORY.len_lists);
    List* list = &MEMORY.lists[MEMORY.len_lists++];
    list->key_chars = key_chars;
    list->key_len = key_len;
    list->value = value;
    if (!scope->list) {
        scope->list = list;
        return;
    }
    list->next = scope->list;
    scope->list = list;
    return;
}

void scope_update(Scope* scope, char* key_chars, u64 key_len, i64 value) {
    while (scope) {
        List* list = scope->list;
        while (list) {
            if (EQ(key_chars, key_len, list->key_chars, list->key_len)) {
                list->value = value;
                return;
            }
            list = list->next;
        }
        scope = scope->parent;
    }
    EXIT();
}

static void print_scopes(Scope* scope) {
    putchar('\n');
    u32 indent = 0;
    while (scope) {
        for (u32 _ = 0; _ < indent; ++_) {
            putchar(' ');
        }
        printf("{\n");
        List* list = scope->list;
        while (list) {
            for (u32 _ = 0; _ < (indent + 2); ++_) {
                putchar(' ');
            }
            printf("%.*s: %ld\n",
                   (i32)list->key_len,
                   list->key_chars,
                   list->value);
            list = list->next;
        }
        scope = scope->parent;
        for (u32 _ = 0; _ < indent; ++_) {
            putchar(' ');
        }
        printf("}\n");
        indent += 2;
    }
}

i32 main(void) {
    memory_init();

    Scope* parent = scope_new();
    scope_insert(parent, "x", 1, -123);
    scope_insert(parent, "y", 1, 456);
    putchar('\n');
    printf("parent.x : %ld\n", scope_lookup(parent, "x", 1));
    printf("parent.y : %ld\n", scope_lookup(parent, "y", 1));
    print_scopes(parent);

    Scope* child = scope_new_from(parent);
    scope_insert(child, "x", 1, 7890);
    scope_update(child, "y", 1, -456);
    putchar('\n');
    printf("parent.x : %ld\n", scope_lookup(parent, "x", 1));
    printf("child.x  : %ld\n", scope_lookup(child, "x", 1));
    printf("parent.y : %ld\n", scope_lookup(parent, "y", 1));
    printf("child.y  : %ld\n", scope_lookup(child, "y", 1));
    print_scopes(child);

    return OK;
}
