#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t   usize;

typedef int32_t i32;

typedef float  f32;
typedef double f64;

#define EXIT_IF(condition)         \
    if (condition) {               \
        fprintf(stderr,            \
                "%s:%s:%d `%s`\n", \
                __FILE__,          \
                __func__,          \
                __LINE__,          \
                #condition);       \
        exit(EXIT_FAILURE);        \
    }

#define PI 3.1415926535897932385f

static f32 get_mean(f32* xs, usize n) {
    EXIT_IF(n == 0);
    f32 sum = 0.0f;
    for (usize i = 0; i < n; ++i) {
        sum += xs[i];
    }
    return sum / (f32)n;
}

static f32 get_variance(f32* xs, usize n) {
    EXIT_IF(n <= 1);
    f32 mean = get_mean(xs, n);
    f32 sum = 0.0f;
    for (usize i = 0; i < n; ++i) {
        f32 delta = xs[i] - mean;
        sum += delta * delta;
    }
    return sum / (f32)n;
}

static f32 get_std(f32* xs, usize n) {
    return sqrtf(get_variance(xs, n));
}

static f32 get_gaussian_pdf(f32 mu, f32 sigma, f32 x) {
    EXIT_IF(sigma <= 0.0f);
    f32 two_sigma_squared = sigma * sigma * 2.0f;
    f32 delta = x - mu;
    return (1 / sqrtf(PI * two_sigma_squared)) *
           (expf(-((delta * delta) / two_sigma_squared)));
}

i32 main(void) {
    f32   xs[] = {0.0f, 1.0f, 2.0f, 2.5f, 3.0f, 4.0f, 5.0f};
    usize n = sizeof(xs) / sizeof(xs[0]);
    f32   mu = get_mean(xs, n);
    f32   sigma = get_std(xs, n);
    printf("mu    : %.5f\n"
           "sigma : %.5f\n",
           (f64)mu,
           (f64)sigma);
    {
        // NOTE: $ R
        //       > print(dnorm(xs, mu, sigma))
        printf("[ ");
        for (usize i = 0; i < n; ++i) {
            printf("%.5f ", (f64)get_gaussian_pdf(mu, sigma, xs[i]));
        }
        printf("]\n");
    }
    return EXIT_SUCCESS;
}
