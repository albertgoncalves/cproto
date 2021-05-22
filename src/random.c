#include <stdint.h>
#include <stdio.h>
#include <sys/time.h>

typedef int32_t i32;

typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;

#define U32_MAX_FLOAT 4294967295.0f

typedef struct timeval timeValue;

typedef struct {
    u32 state;
} xorShiftRng;

typedef struct {
    u64 state;
    u64 increment;
} pcgRng;

static u32 xor_shift_32(xorShiftRng* rng) {
    u32 state = rng->state;
    state ^= state << 13u;
    state ^= state >> 17u;
    state ^= state << 5u;
    rng->state = state;
    return state;
}

static u32 pcg_32(pcgRng* rng) {
    u64 state = rng->state;
    rng->state = (state * 6364136223846793005ull) + (rng->increment | 1u);
    u32 xor_shift = (u32)(((state >> 18u) ^ state) >> 27u);
    u32 rotate = (u32)(state >> 59u);
    return (xor_shift >> rotate) | (xor_shift << ((-rotate) & 31u));
}

static void pcg_32_init(pcgRng* rng, u64 state, u64 increment) {
    rng->state = 0u;
    rng->increment = (increment << 1u) | 1u;
    pcg_32(rng);
    rng->state += state;
    pcg_32(rng);
}

static u64 get_microseconds(void) {
    timeValue time;
    gettimeofday(&time, NULL);
    return (u64)time.tv_usec;
}

i32 main(void) {
    xorShiftRng xor_shift_rng;
    u64         time = get_microseconds();
    xor_shift_rng.state = (u32)time;
    pcgRng pcg_rng;
    pcg_32_init(&pcg_rng, time + 1, time + 2);
    {
        u32 n = 10000000;
        f32 m = (f32)n;
        f32 xor_shift_mean = 0.0f;
        f32 pcg_mean = 0.0f;
        for (u32 _ = 0; _ < n; ++_) {
            {
                u32 x = xor_shift_32(&xor_shift_rng);
                xor_shift_mean += ((f32)x) / U32_MAX_FLOAT;
            }
            {
                u32 x = pcg_32(&pcg_rng);
                pcg_mean += ((f32)x) / U32_MAX_FLOAT;
            }
        }
        printf("iterations          : %u\n"
               "\n"
               "xor_shift_rng.state : %u\n"
               "xor_shift_mean      : %.8f\n\n"
               "pcg_rng.state       : %lu\n"
               "pcg_rng.increment   : %lu\n"
               "pcg_mean            : %.8f\n",
               n,
               xor_shift_rng.state,
               (xor_shift_mean / m),
               pcg_rng.state,
               pcg_rng.increment,
               (pcg_mean / m));
    }
}
