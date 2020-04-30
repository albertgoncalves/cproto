#include <stdio.h>
#include <stdlib.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef float  f32;
typedef double f64;

#define BMP_WIDTH  512
#define BMP_HEIGHT 512
#define BMP_BYTES  1048576

#define WIDTH          512.0f
#define HEIGHT         512.0f
#define HALF_PERIMETER 1024.0f

#define CLAMP 255.0f

#define FILEPATH "out/hello.bmp"

#pragma pack(1)
typedef struct {
    u16 id;
    u32 size;
    u32 _;
    u32 offset;
} bmpHeader;

#pragma pack(1)
typedef struct {
    u32 size;
    u32 width;
    u32 height;
    u16 planes;
    u16 bits_per_pixel;
    u32 _[6];
} dibHeader;

typedef struct {
    bmpHeader bmp_header;
    dibHeader dib_header;
    u8        pixels[BMP_BYTES];
} bmpFile;

static void set_bmp_header(bmpHeader* header) {
    header->id = 0x4d42;
    header->size = sizeof(bmpFile);
    header->offset = sizeof(bmpHeader) + sizeof(dibHeader);
}

static void set_dib_header(dibHeader* header) {
    header->size = sizeof(dibHeader);
    header->width = BMP_WIDTH;
    header->height = BMP_HEIGHT;
    header->planes = 1;
    header->bits_per_pixel = sizeof(u32) * 8;
}

static void set_pixels(u8* pixels) {
    u32 i = 0;
    for (f32 y = 0.0f; y < HEIGHT; ++y) {
        f32 red = (y / HEIGHT) * CLAMP;
        for (f32 x = 0.0f; x < WIDTH; ++x) {
            f32 green = (x / WIDTH) * CLAMP;
            f32 blue = ((x + y) / HALF_PERIMETER) * CLAMP;
            pixels[i] = (u8)blue;
            pixels[i + 1] = (u8)green;
            pixels[i + 2] = (u8)red;
            i += 4;
        }
    }
}

int main(void) {
    FILE* file = fopen(FILEPATH, "wb");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }
    bmpFile* bmp = calloc(sizeof(bmpFile), 1);
    if (bmp == NULL) {
        exit(EXIT_FAILURE);
    }
    set_bmp_header(&bmp->bmp_header);
    set_dib_header(&bmp->dib_header);
    set_pixels(bmp->pixels);
    fwrite(bmp, sizeof(bmpFile), 1, file);
    fclose(file);
    free(bmp);
    return EXIT_SUCCESS;
}
