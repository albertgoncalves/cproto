#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <openssl/sha.h>

typedef int32_t i32;

typedef uint8_t  u8;
typedef uint32_t u32;

typedef size_t usize;

typedef struct {
    u8* buffer;
    u32 len;
} String;

#define OK 0

#if 1
    #define ERROR 1
    #define EXIT_IF(condition)             \
        do {                               \
            if (condition) {               \
                fprintf(stderr,            \
                        "%s:%s:%d `%s`\n", \
                        __FILE__,          \
                        __func__,          \
                        __LINE__,          \
                        #condition);       \
                _exit(ERROR);              \
            }                              \
        } while (0)
#else
    #define EXIT_IF(condition) \
        do {                   \
            (void)(condition); \
        } while (0)
#endif

#define MAGIC "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

#define TABLE                    \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
    "abcdefghijklmnopqrstuvwxyz" \
    "0123456789"                 \
    "+/"

#define PADDING '='

#define MASK0 0xFC0000u
#define MASK1 0x03F000u
#define MASK2 0x000FC0u
#define MASK3 0x00003Fu

#define CAP_BUFFER (1 << 8)
static u8    BUFFER[CAP_BUFFER];
static usize LEN_BUFFER = 0;

static u8* alloc(usize n) {
    u8* pointer = &BUFFER[LEN_BUFFER];
    LEN_BUFFER += n;
    EXIT_IF(CAP_BUFFER <= LEN_BUFFER);
    return pointer;
}

static String concat(const char* input) {
    const u32 n = (u32)strlen(input);

#define M (sizeof(MAGIC) - 1u)
    u8* buffer = alloc(n + M);

    EXIT_IF(!buffer);
    memcpy(buffer, input, n);
    memcpy(&buffer[n], MAGIC, M);
    return (String){
        .buffer = buffer,
        .len = n + M,
    };
#undef M
}

static String encode(const u8* input, u32 n) {
    String string = (String){
        .buffer = alloc((((n + 2u) / 3u) * 4u) + 1u),
        .len = 0,
    };

#define PUSH_BUFFER(c)                   \
    do {                                 \
        string.buffer[string.len++] = c; \
    } while (0)

    for (u32 i = n; i != 0;) {
        const u32 j = n - i;

        if (i == 1) {
            const u32 bits = ((u32)(input[j])) << 16u;

            PUSH_BUFFER(TABLE[(MASK0 & bits) >> 18u]);
            PUSH_BUFFER(TABLE[(MASK1 & bits) >> 12u]);

            PUSH_BUFFER(PADDING);
            PUSH_BUFFER(PADDING);
            break;
        }

        if (i == 2) {
            const u32 bits =
                (((u32)(input[j])) << 16u) | (((u32)(input[j + 1])) << 8u);

            PUSH_BUFFER(TABLE[(MASK0 & bits) >> 18u]);
            PUSH_BUFFER(TABLE[(MASK1 & bits) >> 12u]);
            PUSH_BUFFER(TABLE[(MASK2 & bits) >> 6u]);

            PUSH_BUFFER(PADDING);
            break;
        }

        const u32 bits = (((u32)(input[j])) << 16u) |
                         (((u32)(input[j + 1])) << 8u) |
                         (((u32)(input[j + 2])));

        PUSH_BUFFER(TABLE[(MASK0 & bits) >> 18u]);
        PUSH_BUFFER(TABLE[(MASK1 & bits) >> 12u]);
        PUSH_BUFFER(TABLE[(MASK2 & bits) >> 6u]);
        PUSH_BUFFER(TABLE[MASK3 & bits]);

        i -= 3;
    }
    PUSH_BUFFER('\0');
#undef PUSH_BUFFER

    return string;
}

i32 main(i32 n, const char** args) {
    EXIT_IF(n != 2);
    const String input = concat(args[1]);

    String digest = (String){
        .buffer = alloc(SHA_DIGEST_LENGTH),
        .len = SHA_DIGEST_LENGTH,
    };
    EXIT_IF(digest.buffer != SHA1(input.buffer, input.len, digest.buffer));

    String output = encode(digest.buffer, SHA_DIGEST_LENGTH);
    printf("%.*s\n", (i32)output.len, output.buffer);

    return OK;
}
