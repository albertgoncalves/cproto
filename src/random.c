#include <stdio.h>
#include <sys/time.h>

typedef unsigned int  u32;
typedef unsigned long u64;

typedef float f32;

typedef struct timeval timeValue;

#define U32_MAX_FLOAT 4294967295.0f

#define PCG_CONSTANT 0x853c49e6748fea9bull

typedef struct {
    u64 state;
    u64 increment;
} pcgRng;

typedef struct {
    u32 state;
} xorShiftRng;

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

static u32 get_microseconds(void) {
    timeValue time;
    gettimeofday(&time, NULL);
    return (u32)time.tv_usec;
}

int main(void) {
    xorShiftRng xor_shift_rng;
    xor_shift_rng.state = get_microseconds();
    pcgRng pcg_rng;
    pcg_rng.state = PCG_CONSTANT * get_microseconds();
    pcg_rng.increment = PCG_CONSTANT * get_microseconds();
    {
        u32 n = 100000u;
        f32 m = (f32)n;
        f32 xor_shift_mean = 0.0f;
        f32 pcg_mean = 0.0f;
        for (u32 _ = 0u; _ < n; ++_) {
            xor_shift_mean +=
                ((f32)xor_shift_32(&xor_shift_rng)) / U32_MAX_FLOAT;
            pcg_mean += ((f32)pcg_32(&pcg_rng)) / U32_MAX_FLOAT;
        }
        printf("\n"
               "xorShiftRng.state  : %u\n"
               "xorShiftRng (mean) : %.8f\n\n"
               "pcgRng.state       : %lu\n"
               "pcgRng.increment   : %lu\n"
               "pcgRng      (mean) : %.8f\n",
               xor_shift_rng.state,
               (double)(xor_shift_mean / m),
               pcg_rng.state,
               pcg_rng.increment,
               (double)(pcg_mean / m));
    }
}
