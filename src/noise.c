#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;

typedef FILE fileHandle;

#define PCG_INIT_STATE     0x853C49E6748FEA9Bull
#define PCG_INIT_INCREMENT 0xDA3E39CB94B95BDBull

#define WIDTH  128u
#define HEIGHT 128u
#define SIZE   16384u

#define FILEPATH "out/noise.bmp"

#pragma pack(push, 1u)

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
    u8  _[24u];
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
    header->id = 0x4D42u;
    header->file_size = BMP_FILE_SIZE;
    header->header_offset = BMP_HEADER_SIZE;
}

static void set_dib_header(dibHeader* header) {
    header->header_size = sizeof(dibHeader);
    header->pixel_width = WIDTH;
    header->pixel_height = HEIGHT;
    header->color_planes = 1u;
    header->bits_per_pixel = sizeof(u32) * 8u;
}

__attribute__((no_sanitize("integer"))) static u32 pcg_32(pcgRng* rng) {
    u64 state = rng->state;
    rng->state = (state * 6364136223846793005ull) + (rng->increment | 1u);
    u32 xor_shift = (u32)(((state >> 18u) ^ state) >> 27u);
    u32 rotate = (u32)(state >> 59u);
    return (xor_shift >> rotate) | (xor_shift << ((-rotate) & 31u));
}

__attribute__((no_sanitize("integer"))) static u32 pcg_32_bound(pcgRng* rng,
                                                                u32 bound) {
    u32 threshold = (-bound) % bound;
    for (;;) {
        u32 value = pcg_32(rng);
        if (threshold <= value) {
            return value % bound;
        }
    }
}

static void set_pixels(pixel* pixels, pcgRng* rng) {
    for (u8 y = 0u; y < HEIGHT; ++y) {
        for (u8 x = 0u; x < WIDTH; ++x) {
            u8 value = (u8)pcg_32_bound(rng, 255u);
            pixels->red = value;
            pixels->green = value;
            pixels->blue = value;
            ++pixels;
        }
    }
}

static void write_bmp(fileHandle* file, bmpBuffer* buffer) {
    if (fwrite(&buffer->bmp_header, 1u, sizeof(bmpHeader), file) !=
        sizeof(bmpHeader))
    {
        exit(EXIT_FAILURE);
    }
    if (fwrite(&buffer->dib_header, 1u, sizeof(dibHeader), file) !=
        sizeof(dibHeader))
    {
        exit(EXIT_FAILURE);
    }
    if (fwrite(&buffer->pixels, 1u, sizeof(pixel[SIZE]), file) !=
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
    bmpBuffer* buffer = calloc(1u, sizeof(bmpBuffer));
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }
    set_bmp_header(&buffer->bmp_header);
    set_dib_header(&buffer->dib_header);
    pcgRng rng = {
        .state = PCG_INIT_STATE,
        .increment = PCG_INIT_INCREMENT,
    };
    set_pixels(buffer->pixels, &rng);
    write_bmp(file, buffer);
    fclose(file);
    free(buffer);
    return EXIT_SUCCESS;
}
