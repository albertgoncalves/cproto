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
    buffer[size] = 0;
    return buffer;
}

static tokens_t* get_tokens(const char* buffer) {
    tokens_t* tokens = malloc(sizeof(tokens_t));
    if (tokens == NULL) {
        exit(EXIT_FAILURE);
    }
    uint8_t i_b = 0;
    uint8_t i_t = 0;
    uint8_t i_s = 0;
    for (char c = buffer[i_b]; c != 0;) {
        if (TOKENS_SIZE <= i_t) {
            exit(EXIT_FAILURE);
        }
        if (IS_SPACE(c)) {
            if ((i_b == 0) || (!IS_SPACE(buffer[i_b - 1]))) {
                tokens->items[i_t++].type = SPACE;
            }
            c = buffer[++i_b];
        } else if (IS_ALPHA(c)) {
            if (STRING_BUFFER_LIMIT <= i_s) {
                exit(EXIT_FAILURE);
            }
            tokens->items[i_t].type = WORD;
            tokens->items[i_t++].string = &tokens->strings[i_s];
            tokens->strings[i_s++] = c;
            c = buffer[++i_b];
            while (IS_ALPHA(c)) {
                if (STRING_BUFFER_LIMIT <= i_s) {
                    exit(EXIT_FAILURE);
                }
                tokens->strings[i_s++] = c;
                c = buffer[++i_b];
            }
            tokens->strings[i_s++] = 0;
        } else {
            if (STRING_BUFFER_LIMIT <= i_s) {
                exit(EXIT_FAILURE);
            }
            tokens->items[i_t].type = OTHER;
            tokens->items[i_t++].string = &tokens->strings[i_s];
            tokens->strings[i_s++] = c;
            tokens->strings[i_s++] = 0;
            c = buffer[++i_b];
        }
    }
    for (; i_t < TOKENS_SIZE; ++i_t) {
        tokens->items[i_t].type = EMPTY;
    }
    return tokens;
}

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
