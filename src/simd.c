#include <immintrin.h>
#include <stdio.h>

typedef float  f32;
typedef __m128 lane_f32;

/* NOTE: See `https://software.intel.com/sites/landingpage/IntrinsicsGuide/`.
 */

int main(void) {
    float input[] = {
        1.0f,
        2.0f,
        3.0f,
        4.0f,
        5.0f,
        6.0f,
        7.0f,
        8.0f,
    };
    float output[4];
    lane_f32 a = _mm_setr_ps(input[3], input[2], input[1], input[0]);
    lane_f32 b = _mm_broadcast_ss(&input[4]);
    _mm_storeu_ps((float*)&output, _mm_mul_ps(a, b));
    printf("%.2f\n"
           "%.2f\n"
           "%.2f\n"
           "%.2f\n",
           (double)output[3],
           (double)output[2],
           (double)output[1],
           (double)output[0]);
    return EXIT_SUCCESS;
}
