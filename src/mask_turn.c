#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef int8_t  i8;
typedef int32_t i32;

typedef uint32_t u32;

typedef struct {
    i8 x, y;
} Direction;

// { 1,  0 } => right
// { 0,  1 } => down
// { 0, -1 } => up
// {-1,  0 } => left

static Direction turn(const Direction direction, const bool right) {
    return (Direction){
        (i8)(direction.y * (direction.y != 0) * (-right + (!right))),
        (i8)(direction.x * (direction.x != 0) * (-(!right) + right)),
    };
}

i32 main(void) {
    printf("Done!\n");

    Direction direction = {1, 0};

    for (u32 i = 0; i < 4; ++i) {
        direction = turn(direction, true);
        printf("%hhd, %hhd\n", direction.x, direction.y);
    }

    for (u32 i = 0; i < 4; ++i) {
        direction = turn(direction, false);
        printf("%hhd, %hhd\n", direction.x, direction.y);
    }

    return 0;
}
