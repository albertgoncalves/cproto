#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;

int main(void) {
    {
        const char* text = "Hi!\0";

        putchar('[');
        for (int i = 0; i < 3; ++i) {
            printf("%hhd, ", text[i]);
        }
        printf("%hhd]\n", text[3]);

        const u32* decimal = (const u32*)text;
        printf("%u\n", *decimal);
    }
    {
        const u32   decimal = 2189640u;
        const char* text = (const char*)&decimal;

        putchar('[');
        for (int i = 0; i < 3; ++i) {
            printf("%hhd, ", text[i]);
        }
        printf("%hhd]\n", text[3]);

        printf("%s\n", text);
    }
}
