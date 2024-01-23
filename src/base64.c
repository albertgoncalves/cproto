#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef int32_t i32;

typedef uint8_t  u8;
typedef uint32_t u32;

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

#define TABLE                    \
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
    "abcdefghijklmnopqrstuvwxyz" \
    "0123456789"                 \
    "+/"

#define PADDING '='

#define CAP_BUFFER (1 << 8)
static u8  BUFFER[CAP_BUFFER];
static u32 LEN_BUFFER = 0;

#define PUSH_BUFFER(c)            \
    do {                          \
        BUFFER[LEN_BUFFER++] = c; \
    } while (0)

#define MASK0 0xFC0000u
#define MASK1 0x03F000u
#define MASK2 0x000FC0u
#define MASK3 0x00003Fu

static void encode(const u8* input, u32 n) {
    const u32 m = (((n + 2u) / 3u) * 4u) + 1u;
    EXIT_IF(CAP_BUFFER < m);

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
    EXIT_IF(m != LEN_BUFFER);
}

i32 main(i32 n, const char** args) {
    {
        EXIT_IF(n != 2);
        const char* input = args[1];
        encode((const u8*)input, (u32)strlen(input));
        printf("%s\n", BUFFER);
    }

    {
        LEN_BUFFER = 0;
        const u8 input[] = {
            0xB3, 0x7A, 0x4F, 0x2C, 0xC0, 0x62, 0x4F, 0x16, 0x90, 0xF6,
            0x46, 0x06, 0xCF, 0x38, 0x59, 0x45, 0xB2, 0xBE, 0xC4, 0xEA,
        };
        encode(input, sizeof(input) - 1);
        printf("%s\n", BUFFER);
    }

    return OK;
}
