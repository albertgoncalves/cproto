#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef int32_t  u32;

typedef float f32;

typedef FILE fileHandle;

#define WIDTH  512
#define HEIGHT 512
#define SIZE   262144

#define FLOAT_WIDTH          512.0f
#define FLOAT_HEIGHT         512.0f
#define FLOAT_HALF_PERIMETER 1024.0f

#define SCALE 256.0f

#define FILEPATH "out/hello.bmp"

#pragma pack(push, 1)

typedef struct {
    u16 id;
    u32 file_size;
    u32 _;
    u32 header_offset;
} bmpHeader;

typedef struct {
    u32 header_size;
    u32 pixel_width;
    u32 pixel_height;
    u16 color_planes;
    u16 bits_per_pixel;
    u8  _[24];
} dibHeader;

typedef struct {
    u8 blue;
    u8 green;
    u8 red;
    u8 _;
} pixel;

#define BMP_HEADER_SIZE sizeof(bmpHeader) + sizeof(dibHeader)
#define BMP_FILE_SIZE   BMP_HEADER_SIZE + sizeof(pixel[SIZE])

#pragma pack(pop)

typedef struct {
    pixel     pixels[SIZE];
    dibHeader dib_header;
    bmpHeader bmp_header;
} bmpBuffer;

static void set_bmp_header(bmpHeader* header) {
    header->id = 0x4d42;
    header->file_size = BMP_FILE_SIZE;
    header->header_offset = BMP_HEADER_SIZE;
}

static void set_dib_header(dibHeader* header) {
    header->header_size = sizeof(dibHeader);
    header->pixel_width = WIDTH;
    header->pixel_height = HEIGHT;
    header->color_planes = 1;
    header->bits_per_pixel = sizeof(u32) * 8;
}

static void set_pixels(pixel* pixels) {
    for (f32 y = 0.0f; y < FLOAT_HEIGHT; ++y) {
        f32 red = (y / FLOAT_HEIGHT) * SCALE;
        for (f32 x = 0.0f; x < FLOAT_WIDTH; ++x) {
            f32 green = (x / FLOAT_WIDTH) * SCALE;
            f32 blue = ((x + y) / FLOAT_HALF_PERIMETER) * SCALE;
            pixels->blue = (u8)blue;
            pixels->green = (u8)green;
            pixels->red = (u8)red;
            ++pixels;
        }
    }
}

static void write_bmp(fileHandle* file, bmpBuffer* buffer) {
    if (fwrite(&buffer->bmp_header, 1, sizeof(bmpHeader), file) !=
        sizeof(bmpHeader))
    {
        exit(EXIT_FAILURE);
    }
    if (fwrite(&buffer->dib_header, 1, sizeof(dibHeader), file) !=
        sizeof(dibHeader))
    {
        exit(EXIT_FAILURE);
    }
    if (fwrite(&buffer->pixels, 1, sizeof(pixel[SIZE]), file) !=
        sizeof(pixel[SIZE]))
    {
        exit(EXIT_FAILURE);
    }
}

int main(void) {
    fileHandle* file = fopen(FILEPATH, "wb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    bmpBuffer* buffer = calloc(1, sizeof(bmpBuffer));
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }
    set_bmp_header(&buffer->bmp_header);
    set_dib_header(&buffer->dib_header);
    set_pixels(buffer->pixels);
    write_bmp(file, buffer);
    fclose(file);
    free(buffer);
    return EXIT_SUCCESS;
}
