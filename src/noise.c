#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef float f32;

typedef FILE fileHandle;

typedef struct timeval timeValue;

#define PCG_CONSTANT 0x853c49e6748fea9bull

#define WIDTH  128
#define HEIGHT 128
#define SIZE   16384

#define FILEPATH "out/noise.bmp"

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

typedef struct {
    u64 state;
    u64 increment;
} pcgRng;

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

static u32 get_microseconds(void) {
    timeValue time;
    gettimeofday(&time, NULL);
    return (u32)time.tv_usec;
}

static u32 pcg_32(pcgRng* rng) {
    u64 state = rng->state;
    rng->state = (state * 6364136223846793005ull) + (rng->increment | 1);
    u32 xor_shift = (u32)(((state >> 18u) ^ state) >> 27u);
    u32 rotate = (u32)(state >> 59u);
    return (xor_shift >> rotate) | (xor_shift << ((-rotate) & 31));
}

static u32 pcg_32_bound(pcgRng* rng, u32 bound) {
    u32 threshold = -bound % bound;
    for (;;) {
        u32 value = pcg_32(rng);
        if (threshold <= value) {
            return value % bound;
        }
    }
}

static void set_pixels(pixel* pixels, pcgRng* rng) {
    for (u8 y = 0; y < HEIGHT; ++y) {
        for (u8 x = 0; x < WIDTH; ++x) {
            u8 value = (u8)pcg_32_bound(rng, 255);
            pixels->red = value;
            pixels->green = value;
            pixels->blue = value;
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
    bmpBuffer* buffer = calloc(sizeof(bmpBuffer), 1);
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }
    set_bmp_header(&buffer->bmp_header);
    set_dib_header(&buffer->dib_header);
    pcgRng rng;
    rng.state = PCG_CONSTANT * get_microseconds();
    rng.increment = PCG_CONSTANT * get_microseconds();
    set_pixels(buffer->pixels, &rng);
    write_bmp(file, buffer);
    fclose(file);
    free(buffer);
    return EXIT_SUCCESS;
}
