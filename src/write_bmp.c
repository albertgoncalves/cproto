#include <stdio.h>
#include <stdlib.h>

typedef FILE FileHandle;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

typedef float f32;

#define WIDTH  512
#define HEIGHT 512
#define SIZE   1048576

#define FLOAT_WIDTH          512.0f
#define FLOAT_HEIGHT         512.0f
#define FLOAT_HALF_PERIMETER 1024.0f

#define CLAMP 255.0f

#define FILEPATH "out/hello.bmp"

#pragma pack(1)
typedef struct {
    u16 id;
    u32 file_size;
    u32 _;
    u32 header_offset;
} bmpHeader;

#pragma pack(1)
typedef struct {
    u32 header_size;
    u32 pixel_width;
    u32 pixel_height;
    u16 color_planes;
    u16 bits_per_pixel;
} dibHeader;

#pragma pack(1)
typedef struct {
    bmpHeader bmp_header;
    dibHeader dib_header;
    u8        pixels[SIZE];
} bmpFile;

static void set_bmp_header(bmpHeader* header) {
    header->id = 0x4d42;
    header->file_size = sizeof(bmpFile);
    header->header_offset = sizeof(bmpHeader) + sizeof(dibHeader);
}

static void set_dib_header(dibHeader* header) {
    header->header_size = sizeof(dibHeader);
    header->pixel_width = WIDTH;
    header->pixel_height = HEIGHT;
    header->color_planes = 1;
    header->bits_per_pixel = sizeof(u32) * 8;
}

static void set_pixels(u8* pixels) {
    u32 i = 0;
    for (f32 y = 0.0f; y < FLOAT_HEIGHT; ++y) {
        f32 red = (y / FLOAT_HEIGHT) * CLAMP;
        for (f32 x = 0.0f; x < FLOAT_WIDTH; ++x) {
            f32 green = (x / FLOAT_WIDTH) * CLAMP;
            f32 blue = ((x + y) / FLOAT_HALF_PERIMETER) * CLAMP;
            pixels[i] = (u8)blue;
            pixels[i + 1] = (u8)green;
            pixels[i + 2] = (u8)red;
            i += 4;
        }
    }
}

int main(void) {
    FileHandle* file = fopen(FILEPATH, "wb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    bmpFile* bmp = calloc(sizeof(bmpFile), 1);
    if (bmp == NULL) {
        return EXIT_FAILURE;
    }
    set_bmp_header(&bmp->bmp_header);
    set_dib_header(&bmp->dib_header);
    set_pixels(bmp->pixels);
    u32 filesize = sizeof(bmpFile);
    if (fwrite(bmp, 1, filesize, file) != filesize) {
        return EXIT_FAILURE;
    }
    fclose(file);
    free(bmp);
    return EXIT_SUCCESS;
}
