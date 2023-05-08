#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

// NOTE: See `https://gist.github.com/marcetcheverry/991042`.

typedef int32_t  i32;
typedef uint32_t u32;

#define OK    0
#define ERROR 1

#define DATA "Hello, world!\n"
#define LEN  (sizeof(DATA) - 1)

i32 main(i32 n, const char** args) {
    if (n < 2) {
        return ERROR;
    }
    i32 descriptor =
        open(args[1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (descriptor == -1) {
        return ERROR;
    }

    if (ftruncate(descriptor, LEN)) {
        close(descriptor);
        return ERROR;
    }

    char* memory =
        mmap(0, LEN, PROT_READ | PROT_WRITE, MAP_SHARED, descriptor, 0);
    if (memory == MAP_FAILED) {
        close(descriptor);
        return ERROR;
    }

    memcpy(memory, DATA, LEN);

    if (msync(memory, LEN, MS_SYNC)) {
        close(descriptor);
        return ERROR;
    }

    if (munmap(memory, LEN)) {
        close(descriptor);
        return ERROR;
    }

    close(descriptor);
    return OK;
}
