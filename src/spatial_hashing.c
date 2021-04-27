#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

#define COUNT_X 5
#define COUNT_Y 5
#define COUNT_Z 1

#define EPSILON 0.01f

#define COUNT_LISTS 64

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

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) < (b) ? (b) : (a))

typedef struct {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef struct {
    Vec3 bottom_left_front;
    Vec3 top_right_back;
} Cube;

typedef struct {
    u8 x;
    u8 y;
    u8 z;
} Index;

typedef struct {
    Index lower;
    Index upper;
} Range;

typedef struct List List;

struct List {
    Cube* cube;
    List* next;
    List* last;
};

static const Cube CUBES[] = {
    {
        .bottom_left_front = {.x = 2.0f, .y = 1.0f, .z = 0.0f},
        .top_right_back = {.x = 5.0f, .y = 4.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 4.0f, .y = 6.125f, .z = 0.0f},
        .top_right_back = {.x = 6.0f, .y = 9.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 6.0f, .y = 3.0f, .z = 0.0f},
        .top_right_back = {.x = 10.0f, .y = 5.0, .z = 1.0f},
    },
};

#define COUNT_CUBES  (sizeof(CUBES) / sizeof(CUBES[0]))
#define COUNT_TESTED COUNT_CUBES

typedef struct {
    List grid[COUNT_X][COUNT_Y][COUNT_Z];
    List lists[COUNT_LISTS];
    u8   len_lists;
    u8   tested[COUNT_TESTED];
    u8   len_tested;
} Memory;

static List* alloc_list(Memory* memory) {
    EXIT_IF(COUNT_LISTS <= memory->len_lists);
    return &memory->lists[memory->len_lists++];
}

static Cube get_bounds(void) {
    Cube bounds = CUBES[0];
    for (u8 i = 0; i < COUNT_CUBES; ++i) {
        bounds.bottom_left_front.x =
            MIN(bounds.bottom_left_front.x, CUBES[i].bottom_left_front.x);
        bounds.bottom_left_front.y =
            MIN(bounds.bottom_left_front.y, CUBES[i].bottom_left_front.y);
        bounds.bottom_left_front.z =
            MIN(bounds.bottom_left_front.z, CUBES[i].bottom_left_front.z);
        bounds.top_right_back.x =
            MAX(bounds.top_right_back.x, CUBES[i].top_right_back.x);
        bounds.top_right_back.y =
            MAX(bounds.top_right_back.y, CUBES[i].top_right_back.y);
        bounds.top_right_back.z =
            MAX(bounds.top_right_back.z, CUBES[i].top_right_back.z);
    }
    bounds.bottom_left_front.x -= EPSILON;
    bounds.bottom_left_front.y -= EPSILON;
    bounds.bottom_left_front.z -= EPSILON;
    bounds.top_right_back.x += EPSILON;
    bounds.top_right_back.y += EPSILON;
    bounds.top_right_back.z += EPSILON;
    return bounds;
}

static Vec3 get_span(Cube bounds) {
    return (Vec3){
        .x = bounds.top_right_back.x - bounds.bottom_left_front.x,
        .y = bounds.top_right_back.y - bounds.bottom_left_front.y,
        .z = bounds.top_right_back.z - bounds.bottom_left_front.z,
    };
}

static Range get_range(Cube bounds, Vec3 span, Cube cube) {
    return (Range){
        .lower =
            (Index){
                .x = (u8)(
                    ((cube.bottom_left_front.x - bounds.bottom_left_front.x) /
                     span.x) *
                    COUNT_X),
                .y = (u8)(
                    ((cube.bottom_left_front.y - bounds.bottom_left_front.y) /
                     span.y) *
                    COUNT_Y),
                .z = (u8)(
                    ((cube.bottom_left_front.z - bounds.bottom_left_front.z) /
                     span.z) *
                    COUNT_Z),
            },
        .upper =
            (Index){
                .x = (u8)(
                    ((cube.top_right_back.x - bounds.bottom_left_front.x) /
                     span.x) *
                    COUNT_X),
                .y = (u8)(
                    ((cube.top_right_back.y - bounds.bottom_left_front.y) /
                     span.y) *
                    COUNT_Y),
                .z = (u8)(
                    ((cube.top_right_back.z - bounds.bottom_left_front.z) /
                     span.z) *
                    COUNT_Z),
            },
    };
}

static void iter_indices(Cube bounds, Vec3 span) {
    for (u8 i = 0; i < COUNT_CUBES; ++i) {
        Range range = get_range(bounds, span, CUBES[i]);
        for (u8 x = range.lower.x; x <= range.upper.x; ++x) {
            for (u8 y = range.lower.y; y <= range.upper.y; ++y) {
                for (u8 z = range.lower.z; z <= range.upper.z; ++z) {
                    printf("%2hhu %2hhu %2hhu\n", x, y, z);
                }
            }
        }
        printf("\n");
    }
}

i32 main(void) {
    printf("sizeof(Vec3)   : %zu\n"
           "sizeof(Cube)   : %zu\n"
           "sizeof(List)   : %zu\n"
           "sizeof(Memory) : %zu\n\n",
           sizeof(Vec3),
           sizeof(Cube),
           sizeof(List),
           sizeof(Memory));
    Cube bounds = get_bounds();
    Vec3 span = get_span(bounds);
    printf(" %5.2f %5.2f %5.2f\n"
           " %5.2f %5.2f %5.2f\n"
           " %5.2f %5.2f %5.2f\n",
           (f64)bounds.bottom_left_front.x,
           (f64)bounds.bottom_left_front.y,
           (f64)bounds.bottom_left_front.z,
           (f64)bounds.top_right_back.x,
           (f64)bounds.top_right_back.y,
           (f64)bounds.top_right_back.z,
           (f64)span.x,
           (f64)span.y,
           (f64)span.z);
    printf("\n");
    iter_indices(bounds, span);
    return EXIT_SUCCESS;
}
