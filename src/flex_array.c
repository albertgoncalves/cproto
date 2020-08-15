#include <stdio.h>
#include <stdlib.h>

typedef unsigned char  u8;
typedef unsigned short u16;

#define U8_HALF_MAX 0x7F

typedef struct {
    u8  cap;
    u8  len;
    u16 buffer[];
} FlexArray;

#define STRUCT FlexArray
#define TYPE   u16
#define FORMAT "%hu"
#define CREATE create
#define PUSH   push
#define POP    pop
#define PRINT  print

static STRUCT* CREATE(void) {
    STRUCT* array = malloc(sizeof(STRUCT) + sizeof(TYPE));
    if (array == NULL) {
        exit(EXIT_FAILURE);
    }
    array->cap = 1;
    array->len = 0;
    return array;
}

static STRUCT* PUSH(STRUCT* array, TYPE item) {
    if (array->len == array->cap) {
        if (U8_HALF_MAX < array->cap) {
            exit(EXIT_FAILURE);
        }
        array->cap = (u8)(array->cap << 1);
        array = realloc(array, sizeof(STRUCT) + (sizeof(TYPE) * array->cap));
        if (array == NULL) {
            exit(EXIT_FAILURE);
        }
    }
    array->buffer[array->len++] = item;
    return array;
}

static TYPE* POP(STRUCT* array) {
    if (array->len == 0) {
        return NULL;
    }
    return &array->buffer[--array->len];
}

static void PRINT(STRUCT* array) {
    printf("\n{\n"
           "    cap    : %hhu\n"
           "    len    : %hhu\n"
           "    buffer : [",
           array->cap,
           array->len);
    if (array->len != 0) {
        printf(FORMAT, array->buffer[0]);
        for (u8 i = 1; i < array->len; ++i) {
            printf(", " FORMAT, array->buffer[i]);
        }
    }
    printf("]\n}\n");
}

#undef STRUCT
#undef T
#undef FORMAT
#undef CREATE
#undef PUSH
#undef POP
#undef PRINT

int main(void) {
    FlexArray* array = create();
    printf("sizeof(FlexArray)      : %zu\n"
           "sizeof(u16)            : %zu\n"
           "sizeof(array)          : %zu\n"
           "sizeof(*array->buffer) : %zu\n",
           sizeof(FlexArray),
           sizeof(u16),
           sizeof(array),
           sizeof(*array->buffer));
    for (u8 i = 0; i < 3; ++i) {
        array = push(array, (u8)(i + 1));
        print(array);
    }
    for (;;) {
        u16* item = pop(array);
        if (item != NULL) {
            printf("\npop(array) : %hu", *item);
        } else {
            printf("\n");
            break;
        }
    }
    print(array);
    free(array);
    return EXIT_SUCCESS;
}
