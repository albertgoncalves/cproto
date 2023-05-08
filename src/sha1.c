#include <stdio.h>
#include <string.h>

#include <openssl/sha.h>

typedef int32_t i32;

typedef uint8_t u8;

#define OK    0
#define ERROR 1

// NOTE: See `https://pragmaticjoe.gitlab.io/posts/2015-02-09-how-to-generate-a-sha1-hash-in-c/`.

i32 main(i32 n, const char** args) {
    if (n < 2) {
        return ERROR;
    }

    u8 digest[SHA_DIGEST_LENGTH] = {0};
    SHA1((const u8*)args[1], strlen(args[1]), digest);

    u8 buffer[SHA_DIGEST_LENGTH * 2 + 1] = {0};
    for (i32 i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf((char*)&(buffer[i * 2]), "%02x", digest[i]);
    }
    // NOTE: `$ echo -n ... | sha1sum`
    printf("SHA1 of \"%s\": %s\n", args[1], buffer);

    return OK;
}
