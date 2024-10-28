#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {
    const int pagesize = getpagesize();

    void* pages = mmap(NULL,
                       (size_t)(pagesize * 3),
                       PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_PRIVATE,
                       -1,
                       0);
    if (pages == MAP_FAILED) {
        return 1;
    }

    // NOTE: Let's make it so we can't read or write to the first and last pages.
    void* first_page = mmap(pages,
                            (size_t)pagesize,
                            PROT_NONE,
                            MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                            -1,
                            0);
    if (first_page == MAP_FAILED) {
        return 1;
    }

    void* last_page = mmap(pages + (pagesize * 2),
                           (size_t)pagesize,
                           PROT_NONE,
                           MAP_ANONYMOUS | MAP_PRIVATE | MAP_FIXED,
                           -1,
                           0);
    if (last_page == MAP_FAILED) {
        return 1;
    }

    int* data = (int*)(pages + pagesize);

    // NOTE: Let's step into the region mapped to `last_page`, this should cause a segmentation fault.
    const int n = (pagesize / ((int)sizeof(int))) + 1;

    for (int i = 0; i < n; ++i) {
        data[i] = i * 2;
    }

    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }

    printf("%d\n", sum);

    // // NOTE: This also fails.
    // --data;
    // printf("%d\n", *data);

    return 0;
}
