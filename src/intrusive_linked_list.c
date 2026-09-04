// NOTE: See `https://0xax.dev/books/linux-inside/linux-datastructures-1`.

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct List List;

struct List {
    List* next;
    List* prev;
};

typedef struct {
    const char* string;
    List        list;
} Container;

#define CONTAINER_OF(pointer, type, member)                                             \
    ({                                                                                  \
        static_assert(__builtin_types_compatible_p(__typeof__(*(pointer)),              \
                                                   __typeof__(((type*)NULL)->member))); \
        ((type*)(((void*)(pointer)) - offsetof(type, member)));                         \
    })

int main(void) {
    Container containers[3] = {0};

    containers[0].string = "first";
    containers[1].string = "second";
    containers[2].string = "third";

    for (int i = 0; i < 2; ++i) {
        containers[i].list.next = &containers[i + 1].list;
        containers[i + 1].list.prev = &containers[i].list;
    }

    for (List* list = &containers[0].list; list; list = list->next) {
        Container* container = CONTAINER_OF(list, Container, list);
        printf("> %s\n", container->string);
    }

    putchar('\n');

    for (List* list = &containers[2].list; list; list = list->prev) {
        Container* container = CONTAINER_OF(list, Container, list);
        printf("< %s\n", container->string);
    }

    return 0;
}
