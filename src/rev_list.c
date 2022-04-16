#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t  i32;

#define OK    0
#define ERROR 1

#define EXIT_IF(condition)                                                   \
    if (condition) {                                                         \
        printf("%s:%s:%d `%s`\n", __FILE__, __func__, __LINE__, #condition); \
        _exit(ERROR);                                                        \
    }

typedef struct List List;

struct List {
    i32   head;
    List* tail;
};

#define CAP_LIST (1 << 5)
static List LIST[CAP_LIST];
static u32  LEN_LIST = 0;

static List* alloc(i32 head, List* tail) {
    EXIT_IF(CAP_LIST <= LEN_LIST);
    List* list = &LIST[LEN_LIST++];
    list->head = head;
    list->tail = tail;
    return list;
}

static List* rev(List* list) {
    List* tail = NULL;
    while (list) {
        List* _ = list->tail;
        list->tail = tail;
        tail = list;
        list = _;
    }
    return tail;
}

static void show(List* list) {
    putchar('[');
    while (list) {
        printf("%d", list->head);
        if (list->tail) {
            putchar(',');
        }
        list = list->tail;
    }
    printf("]\n");
}

i32 main(void) {
    show(NULL);
    {
        List* list = alloc(0, alloc(1, alloc(2, alloc(3, NULL))));
        show(list);
        list = rev(list);
        show(list);
    }
    return OK;
}
