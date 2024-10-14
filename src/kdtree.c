#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;
typedef float    f32;
typedef double   f64;

#define ERROR()                                                      \
    {                                                                \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        exit(EXIT_FAILURE);                                          \
    }

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

typedef struct {
    f32 _x;
    f32 _y;
    f32 _z;
} Vec3;

typedef enum {
    DIM_X = 0,
    DIM_Y,
    DIM_Z,
    COUNT_DIMS,
} Dim3;

typedef struct Node Node;

struct Node {
    Dim3  dim;
    Vec3  point;
    Node* left;
    Node* right;
};

static Vec3 POINTS[] = {
    {._x = -4.0f, ._y = -4.0f},
    {._x = -3.0f, ._y = 2.0f},
    {._x = -2.0f, ._y = -1.0f},
    {._x = -1.0f, ._y = -5.0f},
    {._x = -1.0f, ._y = 5.0f},
    {._x = 0.0f, ._y = -2.0f},
    {._x = 1.0f, ._y = 1.0f},
    {._x = 2.5f, ._y = 0.0f},
    {._x = 3.0f, ._y = 4.0f},
    {._x = 4.0f, ._y = -3.0f},
    {._x = 6.0f, ._y = 1.0f},
    {._x = 8.0f, ._y = -2.0f},
    {._x = 9.0f, ._y = 7.0f},
    {._x = 10.0f, ._y = 2.0f},
    {._x = 11.0f, ._y = 2.0f},
    {._x = 11.0f, ._y = 5.0f},
    {._x = 12.0f, ._y = -8.0f},
};

#define COUNT_POINTS (sizeof(POINTS) / sizeof(POINTS[0]))
#define COUNT_NODES  COUNT_POINTS

typedef struct {
    Node nodes[COUNT_NODES];
    u32  len_nodes;
} Memory;

static Node* alloc_node(Memory* memory) {
    EXIT_IF(COUNT_NODES <= memory->len_nodes);
    return &memory->nodes[memory->len_nodes++];
}

#define SWAP(i, j)                \
    {                             \
        const Vec3 t = POINTS[i]; \
        POINTS[i] = POINTS[j];    \
        POINTS[j] = t;            \
    }

#define PARTITION(fn, dim)                   \
    static i32 fn(i32 l, i32 r) {            \
        const f32 pivot = POINTS[r]._##dim;  \
        i32       i = l - 1;                 \
        for (i32 j = l; j < r; ++j) {        \
            if (POINTS[j]._##dim <= pivot) { \
                ++i;                         \
                SWAP(i, j);                  \
            }                                \
        }                                    \
        ++i;                                 \
        SWAP(i, r);                          \
        return i;                            \
    }

PARTITION(partition_x, x)
PARTITION(partition_y, y)
PARTITION(partition_z, z)

#define QUICKSORT(fn, dim)                       \
    void fn(i32, i32);                           \
    void fn(i32 l, i32 r) {                      \
        if (l < r) {                             \
            const i32 i = partition_##dim(l, r); \
            if (i != 0) {                        \
                quicksort_##dim(l, i - 1);       \
            }                                    \
            quicksort_##dim(i + 1, r);           \
        }                                        \
    }

QUICKSORT(quicksort_x, x)
QUICKSORT(quicksort_y, y)
QUICKSORT(quicksort_z, z)

static Node* make_tree(Memory* memory, i32 l, i32 r, Dim3 dim) {
    EXIT_IF(r < l);
    if (l == r) {
        Node* node = alloc_node(memory);
        node->point = POINTS[l];
        node->dim = dim;
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    switch (dim) {
    case DIM_X: {
        quicksort_x(l, r);
        break;
    }
    case DIM_Y: {
        quicksort_y(l, r);
        break;
    }
    case DIM_Z: {
        quicksort_z(l, r);
        break;
    }
    case COUNT_DIMS: {
        ERROR();
    }
    default: {
    }
    }
    Node*      node = alloc_node(memory);
    const i32  m = ((r - l) / 2) + l;
    const Dim3 next_dim = (dim + 1) % COUNT_DIMS;
    node->point = POINTS[m];
    node->dim = dim;
    if (m != l) {
        node->left = make_tree(memory, l, m - 1, next_dim);
    }
    if (m != r) {
        node->right = make_tree(memory, m + 1, r, next_dim);
    }
    return node;
}

static void show_node(const Node* node) {
    printf("{x:%.2f, y:%.2f, z:%.2f}\n",
           (f64)node->point._x,
           (f64)node->point._y,
           (f64)node->point._z);
}

static void show_tree(const Node* node, u32 n) {
    if (!node) {
        return;
    }
    const u32 m = n + 4;
    show_tree(node->left, m);
    for (u32 _ = 0; _ < n; ++_) {
        printf(" ");
    }
    show_node(node);
    show_tree(node->right, m);
}

static f32 distance_squared(Vec3 a, Vec3 b) {
    const f32 x = (a._x - b._x);
    const f32 y = (a._y - b._y);
    const f32 z = (a._z - b._z);
    return (x * x) + (y * y) + (z * z);
}

#define TRAVERSE(node, point, target, split, radius_squared)            \
    {                                                                   \
        f32 overlap = split - target;                                   \
        overlap *= overlap;                                             \
        if (target < split) {                                           \
            show_within_radius(node->left, point, radius_squared);      \
            if (overlap < radius_squared) {                             \
                show_within_radius(node->right, point, radius_squared); \
            }                                                           \
        } else if (split < target) {                                    \
            show_within_radius(node->right, point, radius_squared);     \
            if (overlap < radius_squared) {                             \
                show_within_radius(node->left, point, radius_squared);  \
            }                                                           \
        } else {                                                        \
            show_within_radius(node->left, point, radius_squared);      \
            show_within_radius(node->right, point, radius_squared);     \
        }                                                               \
    }

static void show_within_radius(const Node* node,
                               Vec3        point,
                               f32         radius_squared) {
    if (!node) {
        return;
    }
    if (distance_squared(point, node->point) < radius_squared) {
        show_node(node);
    } else {
        printf(".\n");
    }
    switch (node->dim) {
    case DIM_X: {
        const f32 target = point._x;
        const f32 split = node->point._x;
        TRAVERSE(node, point, target, split, radius_squared);
        break;
    }
    case DIM_Y: {
        const f32 target = point._y;
        const f32 split = node->point._y;
        TRAVERSE(node, point, target, split, radius_squared);
        break;
    }
    case DIM_Z: {
        const f32 target = point._z;
        const f32 split = node->point._z;
        TRAVERSE(node, point, target, split, radius_squared);
        break;
    }
    case COUNT_DIMS: {
        ERROR();
    }
    default: {
    }
    }
}

#undef TRAVERSE

i32 main(void) {
    printf("sizeof(Vec3)   : %zu\n"
           "sizeof(Node)   : %zu\n"
           "sizeof(Memory) : %zu\n",
           sizeof(Vec3),
           sizeof(Node),
           sizeof(Memory));
    {
        Memory* memory = calloc(1, sizeof(Memory));
        make_tree(memory, 0, COUNT_POINTS - 1, DIM_X);
        EXIT_IF(COUNT_POINTS != memory->len_nodes);
        printf("\n");
        show_tree(&memory->nodes[0], 0);
        printf("\n");
        const f32 radius_squared = 3.0f * 3.0f;
        show_within_radius(&memory->nodes[0], (Vec3){0}, radius_squared);
        free(memory);
    }
    return EXIT_SUCCESS;
}
