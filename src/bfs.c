#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int32_t i32;

typedef uint8_t  u8;
typedef uint32_t u32;

typedef struct {
    u8 x, y;
} Vec2u;

#define WALL '#'

#define MAP_X   20
#define MAP_Y   MAP_X
#define CAP_MAP (MAP_X * MAP_Y)

static bool eq_vec2u(Vec2u a, Vec2u b) {
    return (a.x == b.x) & (a.y == b.y);
}

static void reverse_vec2u(Vec2u array[], u32 len) {
    const u32 k = len >> 1;
    for (u32 i = 0; i < k; ++i) {
        const u32   j = (len - i) - 1;
        const Vec2u x = array[i];
        array[i] = array[j];
        array[j] = x;
    }
}

static void bfs_push(const char map[][MAP_X],
                     Vec2u      stack[],
                     Vec2u      parents[][MAP_X],
                     bool       visited[][MAP_X],
                     u32*       j,
                     Vec2u      parent,
                     Vec2u      child) {
    if ((map[child.y][child.x] == WALL) || visited[child.y][child.x]) {
        return;
    }

    stack[(*j)++] = child;
    parents[child.y][child.x] = parent;
    visited[child.y][child.x] = true;
}

static u32 bfs(const char map[][MAP_X],
               Vec2u      stack[],
               Vec2u      parents[][MAP_X],
               bool       visited[][MAP_X],
               Vec2u      path[],
               Vec2u      start,
               Vec2u      end) {
    u32 i = 0;
    u32 j = 0;

    stack[j++] = start;

    memset(visited, false, sizeof(bool) * CAP_MAP);
    visited[start.y][start.x] = true;

    while (i < j) {
        const Vec2u parent = stack[i++];

        if (eq_vec2u(parent, end)) {
            u32 len_path = 0;

            for (Vec2u cell = parent;;) {
                path[len_path++] = cell;

                if (eq_vec2u(cell, start)) {
                    break;
                }

                cell = parents[cell.y][cell.x];
            }

            reverse_vec2u(path, len_path);

            return len_path;
        }

        const Vec2u v = parent;

        if (0 < v.x) {
            bfs_push(map, stack, parents, visited, &j, parent, (Vec2u){v.x - 1, v.y});
        }
        if (0 < v.y) {
            bfs_push(map, stack, parents, visited, &j, parent, (Vec2u){v.x, v.y - 1});
        }
        if (v.x < (MAP_X - 1)) {
            bfs_push(map, stack, parents, visited, &j, parent, (Vec2u){v.x + 1, v.y});
        }
        if (v.y < (MAP_Y - 1)) {
            bfs_push(map, stack, parents, visited, &j, parent, (Vec2u){v.x, v.y + 1});
        }
    }

    return 0;
}

i32 main(void) {
    // clang-format off
    static const char map[MAP_Y][MAP_X] = {
        "                 #  ",
        "                 #  ",
        "  #  ####  #######  ",
        "  #     #           ",
        "  #     #           ",
        "  #  #  #######  ###",
        "     #  #        #  ",
        "     #  #        #  ",
        "######  ####  ####  ",
        "  #        #        ",
        "  #        #        ",
        "  #  #######  ####  ",
        "              #     ",
        "              #     ",
        "######  #######  ###",
        "  #     #        #  ",
        "  #     #        #  ",
        "  #  #  ####  #  #  ",
        "     #        #     ",
        "     #        #     ",
    };
    // clang-format on

    const Vec2u start = {1, 19};
    const Vec2u end = {19, 1};

    static Vec2u parents[MAP_Y][MAP_X];
    static Vec2u stack[CAP_MAP];
    static bool  visited[MAP_Y][MAP_X];
    static Vec2u path[CAP_MAP];

    static_assert(sizeof(visited) == (sizeof(bool) * CAP_MAP));

    const u32 len_path = bfs(map, stack, parents, visited, path, start, end);
    assert(len_path != 0);

    for (u8 y = 0; y < MAP_Y; ++y) {
        for (u8 x = 0; x < MAP_X; ++x) {
            const Vec2u cell = {x, y};

            if (eq_vec2u(cell, end)) {
                putchar('e');
                continue;
            }

            if (eq_vec2u(cell, start)) {
                putchar('s');
                continue;
            }

            bool found = false;
            for (u32 i = 0; i < len_path; ++i) {
                if (eq_vec2u(cell, path[i])) {
                    found = true;
                    break;
                }
            }

            if (found) {
                assert(map[cell.y][cell.x] != '#');
                putchar('.');
            } else {
                putchar(map[cell.y][cell.x]);
            }
        }
        putchar('\n');
    }

    return 0;
}
