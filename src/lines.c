#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int32_t  i32;

typedef float f32;

typedef FILE fileHandle;

typedef struct timeval timeValue;

#define PCG_CONSTANT 0x853c49e6748fea9bull

#define WIDTH  256
#define HEIGHT 256
#define SIZE   65536

#define LIGHT_GRAY 245
#define DARK_GRAY  40

#define FILEPATH "out/lines.bmp"

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

typedef enum {
    darkGray = 0,
    lightGray,
} color;

typedef struct {
    color     mask[SIZE];
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
    u32 threshold = (-bound) % bound;
    for (;;) {
        u32 value = pcg_32(rng);
        if (threshold <= value) {
            return value % bound;
        }
    }
}

static void set_line(color* mask, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 x_delta = abs(x1 - x0);
    i32 x_sign = x0 < x1 ? 1 : -1;
    i32 y_delta = abs(y1 - y0);
    i32 y_sign = y0 < y1 ? 1 : -1;
    i32 error_a = (y_delta < x_delta ? x_delta : -y_delta) / 2;
    for (;;) {
        mask[(y0 * WIDTH) + x0] = lightGray;
        if ((x0 == x1) && (y0 == y1)) {
            return;
        }
        i32 error_b = error_a;
        if (-x_delta < error_b) {
            error_a -= y_delta;
            x0 += x_sign;
        }
        if (error_b < y_delta) {
            error_a += x_delta;
            y0 += y_sign;
        }
    }
}

static void set_pixels(color* mask, pixel* pixels) {
    for (u32 _ = 0; _ < SIZE; ++_) {
        switch (*mask) {
        case darkGray: {
            pixels->red = DARK_GRAY;
            pixels->green = DARK_GRAY;
            pixels->blue = DARK_GRAY;
            break;
        }
        case lightGray: {
            pixels->red = LIGHT_GRAY;
            pixels->green = LIGHT_GRAY;
            pixels->blue = LIGHT_GRAY;
            break;
        }
        }
        ++pixels;
        ++mask;
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
    pcgRng rng;
    rng.state = PCG_CONSTANT * get_microseconds();
    rng.increment = PCG_CONSTANT * get_microseconds();
    for (u8 i = 0; i < 16; ++i) {
        set_line(buffer->mask,
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH));
    }
    set_pixels(buffer->mask, buffer->pixels);
    write_bmp(file, buffer);
    fclose(file);
    free(buffer);
    return EXIT_SUCCESS;
}
