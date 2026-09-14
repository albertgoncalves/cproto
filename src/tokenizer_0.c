#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int32_t  i32;
typedef uint32_t u32;

typedef struct {
    const char* chars;
    u32         len;
} String;

typedef struct {
    String* strings;
    u32     cap;
    u32     len;
} Array;

#define INIT_ARRAY_CAP 32

static void push(Array* const array, const char* const chars, const u32 len) {
    if (array->cap <= array->len) {
        array->cap *= 2;
        array->strings = (String*)reallocarray(array->strings, array->cap, sizeof(String));
        assert(array->strings);
    }

    array->strings[(array->len)++] = (String){
        chars,
        len,
    };
}

static Array tokenize(const char* const source, const u32 len_source) {
    Array array = {
        (String*)calloc(INIT_ARRAY_CAP, sizeof(String)),
        INIT_ARRAY_CAP,
        0,
    };

    u32 i = 0;
    u32 j = 0;
    for (; j < len_source;) {
        switch (source[j]) {
        case ' ':
        case '\t':
        case '\n':
        case '\r': {
            if (i != j) {
                push(&array, &source[i], j - i);
            }
            ++j;
            i = j;
            continue;
        }
        case ',':
        case '(':
        case ')':
        case '{':
        case '}':
        case ':': {
            if (i != j) {
                push(&array, &source[i], j - i);
            }
            push(&array, &source[j], 1);
            ++j;
            i = j;
            continue;
        }
        default: {
            ++j;
        }
        }
    }

    return array;
}

i32 main(void) {
    const char* source =
        "version 49, 0\n"
        "\n"
        "class final super java/lang/Object Hello {\n"
        "    Hello ()V {\n"
        "            aload           this\n"
        "            invokespecial   java/lang/Object, <init>, ()V\n"
        "            return\n"
        "    }\n"
        "\n"
        "    static main (args [Ljava/lang/String;)V {\n"
        "            iconst          0\n"
        "        {\n"
        "            istore          i\n"
        "        for:\n"
        "            iload           i\n"
        "            aload           args\n"
        "            arraylength\n"
        "            if_icmpge       break\n"
        "            getstatic       java/lang/System, out, Ljava/io/PrintStream;\n"
        "            aload           args\n"
        "            iload           i\n"
        "            aaload\n"
        "            invokevirtual   java/io/PrintStream, println, (Ljava/lang/String;)V\n"
        "            iinc            i, 1\n"
        "            goto            for\n"
        "        }\n"
        "        break:\n"
        "            return\n"
        "    }\n"
        "}\n";

    const Array array = tokenize(source, (u32)strlen(source));

    for (u32 i = 0; i < array.len; ++i) {
        printf("\"%.*s\"\n", (i32)array.strings[i].len, array.strings[i].chars);
    }

    free(array.strings);

    return 0;
}
