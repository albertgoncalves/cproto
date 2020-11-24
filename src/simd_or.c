#include <immintrin.h>
#include <stdint.h>
#include <stdio.h>

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int8_t  i8;
typedef int32_t i32;

typedef __m128i Simd4i32;

// NOTE: See `https://software.intel.com/sites/landingpage/IntrinsicsGuide/`.

i32 main(void) {
    i8 a[] = {1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1};
    i8 b[] = {4, 2, 2, 0, 4, 2, 2, 0, 4, 2, 2, 0, 4, 2, 2, 0};
    i8 c[16] = {0};
    _mm_store_si128((Simd4i32*)&c[0],
                    _mm_or_si128(*(Simd4i32*)&a[0], *(Simd4i32*)&b[0]));
    printf("[");
    for (u8 i = 0; i < 16; ++i) {
        printf(" %hhu", c[i]);
    }
    printf(" ]\n");
    return EXIT_SUCCESS;
}
