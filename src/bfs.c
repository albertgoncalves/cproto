#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef int32_t i32;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef struct {
    u8 x, y;
} Vec2u;

#define WALL '#'

#define MAP_W   20
#define MAP_H   MAP_W
#define CAP_MAP (MAP_W * MAP_H)

static u16 to_index(Vec2u v) {
    return v.x + (v.y * MAP_W);
}

static Vec2u from_index(u16 i) {
    return (Vec2u){
        (u8)(i % MAP_W),
        (u8)(i / MAP_W),
    };
}

static void reverse_u16(u16* array, i32 len) {
    const i32 k = len >> 1;
    for (u16 i = 0; i < k; ++i) {
        const u16 j = (u16)((len - i) - 1);
        const u16 x = array[i];
        array[i] = array[j];
        array[j] = x;
    }
}

static void bfs_push(const char* map,
                     u16*        stack,
                     u16*        parents,
                     bool*       visited,
                     u32*        j,
                     u16         parent,
                     u16         child) {
    if ((map[child] == WALL) || visited[child]) {
        return;
    }

    stack[(*j)++] = child;
    parents[child] = parent;
    visited[child] = true;
}

static i32 bfs(const char* map,
               u16*        stack,
               u16*        parents,
               bool*       visited,
               u16*        path,
               u16         start,
               u16         end) {
    u32 i = 0;
    u32 j = 0;

    stack[j++] = start;

    memset(visited, false, sizeof(bool) * CAP_MAP);
    visited[start] = true;

    while (i < j) {
        const u16 parent = stack[i++];

        if (parent == end) {
            i32 len_path = 0;

            for (u16 cell = parent;;) {
                path[len_path++] = cell;

                if (cell == start) {
                    break;
                }

                cell = parents[cell];
            }

            reverse_u16(path, len_path);

            return len_path;
        }

        const Vec2u v = from_index(parent);

        if (0 < v.x) {
            bfs_push(map, stack, parents, visited, &j, parent, to_index((Vec2u){v.x - 1, v.y}));
        }
        if (0 < v.y) {
            bfs_push(map, stack, parents, visited, &j, parent, to_index((Vec2u){v.x, v.y - 1}));
        }
        if (v.x < (MAP_W - 1)) {
            bfs_push(map, stack, parents, visited, &j, parent, to_index((Vec2u){v.x + 1, v.y}));
        }
        if (v.y < (MAP_H - 1)) {
            bfs_push(map, stack, parents, visited, &j, parent, to_index((Vec2u){v.x, v.y + 1}));
        }
    }

    return -1;
}

i32 main(void) {
    for (u16 i = 0; i < CAP_MAP; ++i) {
        assert(i == to_index(from_index(i)));
    }

    static const char map[CAP_MAP + 1] = "                 #  "
                                         "                 #  "
                                         "  #  ####  #######  "
                                         "  #     #           "
                                         "  #     #           "
                                         "  #  #  #######  ###"
                                         "     #  #        #  "
                                         "     #  #        #  "
                                         "######  ####  ####  "
                                         "  #        #        "
                                         "  #        #        "
                                         "  #  #######  ####  "
                                         "              #     "
                                         "              #     "
                                         "######  #######  ###"
                                         "  #     #        #  "
                                         "  #     #        #  "
                                         "  #  #  ####  #  #  "
                                         "     #        #     "
                                         "     #        #     ";

    const u16 start = to_index((Vec2u){1, 19});
    const u16 end = to_index((Vec2u){19, 1});

    static u16  parents[CAP_MAP];
    static u16  stack[CAP_MAP];
    static bool visited[CAP_MAP];
    static u16  path[CAP_MAP];

    static_assert(sizeof(visited) == (sizeof(bool) * CAP_MAP));

    const i32 len_path = bfs(map, stack, parents, visited, path, start, end);
    assert(len_path != -1);

    for (u8 y = 0; y < MAP_H; ++y) {
        for (u8 x = 0; x < MAP_W; ++x) {
            const u16 cell = to_index((Vec2u){x, y});

            if (cell == end) {
                putchar('e');
                continue;
            }

            if (cell == start) {
                putchar('s');
                continue;
            }

            bool found = false;
            for (i32 i = 0; i < len_path; ++i) {
                if (cell == path[i]) {
                    found = true;
                    break;
                }
            }

            if (found) {
                assert(map[cell] != '#');
                putchar('.');
            } else {
                putchar(map[cell]);
            }
        }
        putchar('\n');
    }

    return 0;
}
