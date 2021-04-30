#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://swtch.com/~rsc/regexp/regexp2.html`.

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

#define COUNT_LABEL 32
#define COUNT_INST  32

typedef struct {
    const char* chars;
    u8          len;
} String;

typedef enum {
    INST_MATCH = 0,
    INST_CHAR,
    INST_JMP,
    INST_SPLIT,
    COUNT_INST_TAG,
} InstTag;

typedef union {
    u8   as_label[2];
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
    u8    labels[COUNT_LABEL];
    Inst  insts[COUNT_INST];
    Index index;
} Memory;

/*
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
*/

#define TO_STRING(literal)          \
    ((String){                      \
        .chars = literal,           \
        .len = sizeof(literal) - 1, \
    })

typedef enum {
    LIST_CHAR = 0,
    LIST_CONCAT,
    LIST_OR,
    LIST_ZERO_OR_ONE,
    LIST_ZERO_OR_MANY,
    LIST_ONE_OR_MANY,
} ListTag;

typedef struct List List;

typedef union {
    List* as_list[2];
    char  as_char;
} ListOp;

struct List {
    ListOp  op;
    ListTag tag;
};

/*           "fo*|bar?"
 *
 *           __ (|) __
 *          /         \
 *        ( )         ( )
 *       /   \       /   \
 *      f    (*)    b    ( )
 *             \        /   \
 *              o      a    (?)
 *                            \
 *                             r
 */

i32 main(void) {
    printf("sizeof(InstTag)  : %zu\n"
           "sizeof(InstOp)   : %zu\n"
           "sizeof(Inst)     : %zu\n"
           "sizeof(Index)    : %zu\n"
           "sizeof(Memory)   : %zu\n\n",
           sizeof(InstTag),
           sizeof(InstOp),
           sizeof(Inst),
           sizeof(Index),
           sizeof(Memory));
    Memory* memory = calloc(1, sizeof(Memory));
    String  regex = TO_STRING("fo*|bar?");
    (void)regex;
    printf("Done!\n");
    free(memory);
    return EXIT_SUCCESS;
}
