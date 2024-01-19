#include <stdint.h>
#include <stdio.h>

typedef uint32_t u32;
typedef int32_t  i32;

#define OK 0

static i32 inner(i32* n) {
    return (*n)++;
}

static i32 (*counter(i32* n))(i32*) {
    *n = 0;
    return inner;
}

/*
    function counter() {
        var n = 0;
        return function() {
            return n++;
        }
    }

    for (var j = 0;;) {
        var c = counter();
        for (var i = 0; i < 5; ++i) {
            console.log(c());
        }
        ++j
        if (3 <= j) {
            break;
        }
        console.log("");
    }
*/

/*
    var n = [null];

    function counter(n) {
        n[0] = 0;
        return function(n) {
            return n[0]++;
        }
    }

    for (var j = 0;;) {
        var c = counter(n);
        for (var i = 0; i < 5; ++i) {
            console.log(c(n));
        }
        ++j
        if (3 <= j) {
            break;
        }
        console.log("");
    }
*/

i32 main(void) {
    i32 n;
    i32 (*f)(i32*);

    for (u32 j = 0;;) {
        f = counter(&n);
        for (u32 i = 0; i < 5; ++i) {
            printf("%u\n", f(&n));
        }
        ++j;
        if (3 <= j) {
            break;
        }
        putchar('\n');
    }

    return OK;
}
