#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;

typedef int32_t i32;

static u16 read_hex_to_u16(const char* hex) {
    u16 result = 0;
    while (*hex) {
        u8  digit = (u8)(*hex++);
        u16 value = 0;
        if (('0' <= digit) && (digit <= '9')) {
            value = (u16)(digit - '0');
        } else if (('a' <= digit) && (digit <= 'f')) {
            value = (u16)((digit - 'a') + 10);
        } else if (('A' <= digit) && (digit <= 'F')) {
            value = (u16)((digit - 'A') + 10);
        }
        result = (u16)((result << 4) | (value & 0xF));
    }
    return result;
}

i32 main(void) {
    printf("Done!\n");
    const char* array[] = {
        "a",
        "A",
        "b",
        "B",
        "f",
        "F",
        "FF",
        "fff",
        "FFFF",
        "57",
    };
    u8 n = (u8)(sizeof(array) / sizeof(char*));
    for (u8 i = 0; i < n; ++i) {
        printf("0x%-4s => %hu\n", array[i], read_hex_to_u16(array[i]));
    }
    return EXIT_SUCCESS;
}
