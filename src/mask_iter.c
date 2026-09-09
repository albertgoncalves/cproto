#include <stdio.h>

#define COLS 4
#define ROWS 3

static void iter(const int mask_x) {
    const int map[ROWS][COLS] = {
        {0, 1, 2, 3},
        {4, 5, 6, 7},
        {8, 9, 10, 11},
    };

    const int mask_y = 1 - mask_x;

    const int max_x = (COLS * mask_x) + (ROWS * mask_y);
    const int max_y = (COLS * mask_y) + (ROWS * mask_x);

    putchar('\n');
    for (int y = 0; y < max_y; ++y) {
        for (int x = 0; x < max_x; ++x) {
            const int j = (x * mask_x) + (y * mask_y);
            const int i = (x * mask_y) + (y * mask_x);

            printf("%3d", map[i][j]);
        }
        putchar('\n');
    }
}

int main(void) {
    iter(1);
    iter(0);

    return 0;
}
