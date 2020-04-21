#include <stdio.h>
#include <stdlib.h>

/* NOTE:
 *  $ runc src/file_stream.c data/hello.txt
 *  Hello, world!
 */

int main(int argv, char** argc) {
    if (argv == 1) {
        return EXIT_FAILURE;
    }
    FILE* file = fopen(argc[1], "r");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    for (int i = fgetc(file); i != EOF; i = fgetc(file)) {
        char c = (char)i;
        printf("%c", c);
    }
    fclose(file);
    return EXIT_SUCCESS;
}
