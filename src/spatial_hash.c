#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

#define COUNT_GRID_X 5
#define COUNT_GRID_Y 3
#define COUNT_GRID_Z 1

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

#define MIN(a, b)     ((a) < (b) ? (a) : (b))
#define MAX(a, b)     ((a) < (b) ? (b) : (a))
#define CLIP(x, l, r) MIN((r), MAX((l), (x)))

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

static Cube CUBES[] = {
    {
        .bottom_left_front = {.x = 2.0f, .y = 1.0f, .z = 0.0f},
        .top_right_back = {.x = 5.0f, .y = 4.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 3.0f, .y = 2.0f, .z = 0.0f},
        .top_right_back = {.x = 5.0f, .y = 8.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 4.0f, .y = 6.125f, .z = 0.0f},
        .top_right_back = {.x = 6.0f, .y = 9.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 6.0f, .y = 3.0f, .z = 0.0f},
        .top_right_back = {.x = 10.0f, .y = 5.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 9.0f, .y = 4.0f, .z = 0.0f},
        .top_right_back = {.x = 10.0f, .y = 5.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 4.0f, .y = 4.0f, .z = 0.0f},
        .top_right_back = {.x = 8.0f, .y = 7.0, .z = 1.0f},
    },
    {
        .bottom_left_front = {.x = 9.0f, .y = 7.0f, .z = 0.0f},
        .top_right_back = {.x = 10.0f, .y = 12.0, .z = 1.0f},
    },
};

#define COUNT_CUBES (sizeof(CUBES) / sizeof(CUBES[0]))

typedef struct List List;

struct List {
    Cube* cube;
    List* next;
    List* last;
};

typedef struct {
    List  grid[COUNT_GRID_X][COUNT_GRID_Y][COUNT_GRID_Z];
    Cube  bounds;
    Vec3  span;
    List  lists[COUNT_LISTS];
    u8    len_lists;
    Cube* intersects[COUNT_CUBES];
    u8    len_intersects;
} Memory;

static List* alloc_list(Memory* memory) {
    EXIT_IF(COUNT_LISTS <= memory->len_lists);
    List* list = &memory->lists[memory->len_lists++];
    list->cube = NULL;
    list->next = NULL;
    list->last = NULL;
    return list;
}

static void set_bounds(Memory* memory) {
    memory->bounds = CUBES[0];
    for (u8 i = 0; i < COUNT_CUBES; ++i) {
        memory->bounds.bottom_left_front.x =
            MIN(memory->bounds.bottom_left_front.x,
                CUBES[i].bottom_left_front.x);
        memory->bounds.bottom_left_front.y =
            MIN(memory->bounds.bottom_left_front.y,
                CUBES[i].bottom_left_front.y);
        memory->bounds.bottom_left_front.z =
            MIN(memory->bounds.bottom_left_front.z,
                CUBES[i].bottom_left_front.z);
        memory->bounds.top_right_back.x =
            MAX(memory->bounds.top_right_back.x, CUBES[i].top_right_back.x);
        memory->bounds.top_right_back.y =
            MAX(memory->bounds.top_right_back.y, CUBES[i].top_right_back.y);
        memory->bounds.top_right_back.z =
            MAX(memory->bounds.top_right_back.z, CUBES[i].top_right_back.z);
    }
    memory->bounds.top_right_back.x += EPSILON;
    memory->bounds.top_right_back.y += EPSILON;
    memory->bounds.top_right_back.z += EPSILON;
}

static void set_span(Memory* memory) {
    memory->span = (Vec3){
        .x = memory->bounds.top_right_back.x -
             memory->bounds.bottom_left_front.x,
        .y = memory->bounds.top_right_back.y -
             memory->bounds.bottom_left_front.y,
        .z = memory->bounds.top_right_back.z -
             memory->bounds.bottom_left_front.z,
    };
}

static Range get_range(Memory* memory, Cube cube) {
    return (Range){
        .lower =
            (Index){
                .x = (u8)(((cube.bottom_left_front.x -
                            memory->bounds.bottom_left_front.x) /
                           memory->span.x) *
                          COUNT_GRID_X),
                .y = (u8)(((cube.bottom_left_front.y -
                            memory->bounds.bottom_left_front.y) /
                           memory->span.y) *
                          COUNT_GRID_Y),
                .z = (u8)(((cube.bottom_left_front.z -
                            memory->bounds.bottom_left_front.z) /
                           memory->span.z) *
                          COUNT_GRID_Z),
            },
        .upper =
            (Index){
                .x = (u8)(((cube.top_right_back.x -
                            memory->bounds.bottom_left_front.x) /
                           memory->span.x) *
                          COUNT_GRID_X),
                .y = (u8)(((cube.top_right_back.y -
                            memory->bounds.bottom_left_front.y) /
                           memory->span.y) *
                          COUNT_GRID_Y),
                .z = (u8)(((cube.top_right_back.z -
                            memory->bounds.bottom_left_front.z) /
                           memory->span.z) *
                          COUNT_GRID_Z),
            },
    };
}

static void push_grid(Memory* memory, Index grid_index, Cube* cube) {
    List* grid = &memory->grid[grid_index.x][grid_index.y][grid_index.z];
    if (!grid->cube) {
        grid->cube = cube;
        grid->next = NULL;
        grid->last = NULL;
        return;
    }
    List* list = alloc_list(memory);
    list->cube = cube;
    if (!grid->next) {
        EXIT_IF(grid->last);
        grid->next = list;
        grid->last = list;
        return;
    }
    grid->last->next = list;
    grid->last = list;
}

static void set_grid(Memory* memory) {
    memset(memory->grid, 0, sizeof(memory->grid));
    memory->len_lists = 0;
    for (u8 i = 0; i < COUNT_CUBES; ++i) {
        Range range = get_range(memory, CUBES[i]);
        for (u8 x = range.lower.x; x <= range.upper.x; ++x) {
            for (u8 y = range.lower.y; y <= range.upper.y; ++y) {
                for (u8 z = range.lower.z; z <= range.upper.z; ++z) {
                    push_grid(memory,
                              ((Index){.x = x, .y = y, .z = z}),
                              &CUBES[i]);
                    printf("%p -> {x:%hhu, y:%hhu, z:%hhu}\n",
                           (void*)&CUBES[i],
                           x,
                           y,
                           z);
                }
            }
        }
        printf("\n");
    }
}

static void show_grid(Memory* memory) {
    for (u8 x = 0; x < COUNT_GRID_X; ++x) {
        for (u8 y = 0; y < COUNT_GRID_Y; ++y) {
            for (u8 z = 0; z < COUNT_GRID_Z; ++z) {
                printf("{x:%hhu, y:%hhu, z:%hhu}\n", x, y, z);
                List* list = &memory->grid[x][y][z];
                while (list && (list->cube)) {
                    printf(" -> %p\n", (void*)list->cube);
                    list = list->next;
                }
                printf("\n");
            }
        }
    }
}

static Cube get_within_bounds(Memory* memory, Cube cube) {
    EXIT_IF((cube.top_right_back.x < cube.bottom_left_front.x) ||
            (cube.top_right_back.y < cube.bottom_left_front.y) ||
            (cube.top_right_back.z < cube.bottom_left_front.z));
    Vec3 top_right_back = (Vec3){
        .x = memory->bounds.top_right_back.x - EPSILON,
        .y = memory->bounds.top_right_back.y - EPSILON,
        .z = memory->bounds.top_right_back.z - EPSILON,
    };
    return (Cube){
        .bottom_left_front =
            {
                .x = CLIP(cube.bottom_left_front.x,
                          memory->bounds.bottom_left_front.x,
                          top_right_back.x),
                .y = CLIP(cube.bottom_left_front.y,
                          memory->bounds.bottom_left_front.y,
                          top_right_back.y),
                .z = CLIP(cube.bottom_left_front.z,
                          memory->bounds.bottom_left_front.z,
                          top_right_back.z),
            },
        .top_right_back =
            {
                .x = CLIP(cube.top_right_back.x,
                          memory->bounds.bottom_left_front.x,
                          top_right_back.x),
                .y = CLIP(cube.top_right_back.y,
                          memory->bounds.bottom_left_front.y,
                          top_right_back.y),
                .z = CLIP(cube.top_right_back.z,
                          memory->bounds.bottom_left_front.z,
                          top_right_back.z),
            },
    };
}

static void set_intersects(Memory* memory, Cube cube) {
    memory->len_intersects = 0;
    Range range = get_range(memory, get_within_bounds(memory, cube));
    for (u8 x = range.lower.x; x <= range.upper.x; ++x) {
        for (u8 y = range.lower.y; y <= range.upper.y; ++y) {
            for (u8 z = range.lower.z; z <= range.upper.z; ++z) {
                List* list = &memory->grid[x][y][z];
                while (list && (list->cube)) {
                    for (u8 i = 0; i < memory->len_intersects; ++i) {
                        if (list->cube == memory->intersects[i]) {
                            goto next;
                        }
                    }
                    EXIT_IF(COUNT_CUBES <= memory->len_intersects);
                    memory->intersects[memory->len_intersects++] = list->cube;
                next:
                    list = list->next;
                }
                printf(".");
            }
        }
    }
    printf("\n\n");
    for (u8 i = 0; i < memory->len_intersects; ++i) {
        printf("%p\n", (void*)memory->intersects[i]);
    }
}

i32 main(void) {
    printf("\n"
           "sizeof(Vec3)   : %zu\n"
           "sizeof(Cube)   : %zu\n"
           "sizeof(Index)  : %zu\n"
           "sizeof(Range)  : %zu\n"
           "sizeof(List)   : %zu\n"
           "sizeof(Memory) : %zu\n"
           "\n",
           sizeof(Vec3),
           sizeof(Cube),
           sizeof(Index),
           sizeof(Range),
           sizeof(List),
           sizeof(Memory));
    {
        Memory* memory = calloc(1, sizeof(Memory));
        set_bounds(memory);
        set_span(memory);
        set_grid(memory);
        show_grid(memory);
        set_intersects(
            memory,
            (Cube){
                .bottom_left_front = {.x = 4.5f, .y = 4.5f, .z = 0.0f},
                .top_right_back = {.x = 5.0f, .y = 5.0f, .z = 1.0f},
            });
        free(memory);
    }
    return EXIT_SUCCESS;
}
