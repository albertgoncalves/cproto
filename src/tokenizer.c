#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_BUFFER_SIZE 128

#define TOKENS_SIZE         8
#define STRING_BUFFER_SIZE  16
#define STRING_BUFFER_LIMIT 15

#define IS_SPACE(c) (c < '!')
#define IS_ALPHA(c) (('A' <= c) && (c <= 'z'))

typedef enum {
    EMPTY = 0,
    SPACE,
    WORD,
    OTHER,
} token_type_t;

typedef struct {
    token_type_t type;
    char*        string;
} token_t;

typedef struct {
    token_t items[TOKENS_SIZE];
    char    strings[STRING_BUFFER_SIZE];
} tokens_t;

static char* get_buffer(FILE* file) {
    fseek(file, 0, SEEK_END);
    size_t size = (size_t)ftell(file);
    if (MAX_FILE_BUFFER_SIZE <= size) {
        exit(EXIT_FAILURE);
    }
    rewind(file);
    char* buffer = malloc((sizeof(char) * size) + 1);
    if (fread(buffer, sizeof(char), size, file) != size) {
        exit(EXIT_FAILURE);
    }
    buffer[size] = '\0';
    return buffer;
}

#define INCREMENT_BUFFER c = buffer[++i_b]
#define SET_POINTER      tokens->items[i_t].string = &tokens->strings[i_s]
#define SET_CHAR(c)      tokens->strings[i_s++] = c
#define SET_TYPE(t)      tokens->items[i_t++].type = t
#define EXIT_IF_STRING_BUFFER_OVERFLOW \
    if (STRING_BUFFER_LIMIT <= i_s) {  \
        exit(EXIT_FAILURE);            \
    }

static tokens_t* get_tokens(const char* buffer) {
    tokens_t* tokens = malloc(sizeof(tokens_t));
    if (tokens == NULL) {
        exit(EXIT_FAILURE);
    }
    uint8_t i_b = 0;
    uint8_t i_t = 0;
    uint8_t i_s = 0;
    for (char c = buffer[i_b]; c != '\0';) {
        if (TOKENS_SIZE == i_t) {
            exit(EXIT_FAILURE);
        }
        if (IS_SPACE(c)) {
            if ((i_b == 0) || (!IS_SPACE(buffer[i_b - 1]))) {
                SET_TYPE(SPACE);
            }
            INCREMENT_BUFFER;
        } else if (IS_ALPHA(c)) {
            /* NOTE: There must be room for the next character *and* '\0'! */
            EXIT_IF_STRING_BUFFER_OVERFLOW;
            SET_POINTER;
            SET_TYPE(WORD);
            SET_CHAR(c);
            INCREMENT_BUFFER;
            while (IS_ALPHA(c)) {
                EXIT_IF_STRING_BUFFER_OVERFLOW;
                SET_CHAR(c);
                INCREMENT_BUFFER;
            }
            SET_CHAR('\0');
        } else {
            EXIT_IF_STRING_BUFFER_OVERFLOW;
            SET_POINTER;
            SET_TYPE(OTHER);
            SET_CHAR(c);
            SET_CHAR('\0');
            INCREMENT_BUFFER;
        }
    }
    for (; i_t < TOKENS_SIZE; ++i_t) {
        tokens->items[i_t].type = EMPTY;
    }
    return tokens;
}

#undef INCREMENT_BUFFER
#undef SET_POINTER
#undef SET_CHAR
#undef SET_TYPE
#undef EXIT_IF_STRING_BUFFER_OVERFLOW

int main(int argv, char** argc) {
    if (argv == 1) {
        return EXIT_FAILURE;
    }
    FILE* file = fopen(argc[1], "r");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    char* buffer = get_buffer(file);
    fclose(file);
    tokens_t* tokens = get_tokens(buffer);
    free(buffer);
    for (uint8_t i = 0; i < TOKENS_SIZE; ++i) {
        token_t* token = &tokens->items[i];
        switch (token->type) {
        case EMPTY: {
            printf("(%2u) _\n", i);
            break;
        }
        case SPACE: {
            printf("(%2u) SPACE\n", i);
            break;
        }
        case WORD: {
            printf("(%2u) WORD  : \"%s\"\n", i, token->string);
            break;
        }
        case OTHER: {
            printf("(%2u) OTHER : \"%s\"\n", i, token->string);
            break;
        }
        }
    }
    free(tokens);
    return EXIT_SUCCESS;
}
