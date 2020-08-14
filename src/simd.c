#include <immintrin.h>
#include <stdio.h>

typedef float  f32;
typedef double f64;
typedef __m128 f32Lane;

/* NOTE: See `https://software.intel.com/sites/landingpage/IntrinsicsGuide/`.
 */

int main(void) {
    f32 input[] = {
        1.0f,
        2.0f,
        3.0f,
        4.0f,
        5.0f,
        6.0f,
        7.0f,
        8.0f,
    };
    f32     output[4];
    f32Lane a = _mm_setr_ps(input[0], input[1], input[2], input[3]);
    f32Lane b = _mm_load_ps(&input[4]);
    _mm_store_ps((f32*)&output, _mm_mul_ps(a, b));
    printf("%4.1f\n"
           "%4.1f\n"
           "%4.1f\n"
           "%4.1f\n",
           (f64)output[0],
           (f64)output[1],
           (f64)output[2],
           (f64)output[3]);
    return EXIT_SUCCESS;
}
