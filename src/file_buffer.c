#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_FILE_BUFFER_SIZE 128
#define TOKEN_BUFFER_SIZE    6
#define TOKEN_BUFFER_LIMIT   5
#define TOKENS_SIZE          8

typedef enum {
    EMPTY = 0,
    SPACE,
    WORD,
    OTHER,
} token_type_t;

typedef struct {
    token_type_t type;
    char         buffer[TOKEN_BUFFER_SIZE];
} token_t;

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

static token_t* get_tokens(const char* buffer) {
    token_t* tokens = calloc(sizeof(token_t), TOKENS_SIZE);
    if (tokens == NULL) {
        exit(EXIT_FAILURE);
    }
    uint8_t b_index = 0;
    uint8_t t_index = 0;
    for (char c = buffer[b_index]; c != 0;) {
        if (TOKENS_SIZE <= t_index) {
            exit(EXIT_FAILURE);
        }
        if (c < '!') {
            tokens[t_index++].type = SPACE;
            c = buffer[++b_index];
        } else if (('A' <= c) && (c <= 'z')) {
            for (uint8_t s_index = 0;; ++s_index) {
                if (TOKEN_BUFFER_LIMIT <= s_index) {
                    exit(EXIT_FAILURE);
                }
                tokens[t_index].type = WORD;
                tokens[t_index].buffer[s_index] = c;
                c = buffer[++b_index];
                if (c == 0) {
                    return tokens;
                }
                if (!(('A' <= c) && (c <= 'z'))) {
                    ++t_index;
                    break;
                }
            }
        } else {
            tokens[t_index].type = OTHER;
            tokens[t_index++].buffer[0] = c;
            c = buffer[++b_index];
        }
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
    token_t* tokens = get_tokens(buffer);
    free(buffer);
    for (uint8_t i = 0; i < TOKENS_SIZE; ++i) {
        token_t* token = &tokens[i];
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
            printf("(%2u) WORD  : \"%s\"\n", i, token->buffer);
            break;
        }
        case OTHER: {
            printf("(%2u) OTHER : \"%s\"\n", i, token->buffer);
            break;
        }
        }
    }
    free(tokens);
    return EXIT_SUCCESS;
}
