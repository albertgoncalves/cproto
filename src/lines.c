#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;
typedef unsigned long  u64;

typedef int i32;

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

#pragma pack(pop)

typedef struct {
    u8 blue;
    u8 green;
    u8 red;
    u8 _;
} pixel;

typedef struct {
    bmpHeader bmp_header;
    dibHeader dib_header;
    pixel     pixels[SIZE];
} bmpFile;

typedef enum {
    darkGray = 0,
    lightGray,
} color;

typedef struct {
    bmpFile bmp_buffer;
    color   mask[SIZE];
} memoryPool;

typedef struct {
    u64 state;
    u64 increment;
} pcgRng;

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

static void set_line(color* mask, i32 x0, i32 y0, i32 x1, i32 y1) {
    i32 x_delta = abs(x1 - x0);
    i32 x_sign = x0 < x1 ? 1 : -1;
    i32 y_delta = abs(y1 - y0);
    i32 y_sign = y0 < y1 ? 1 : -1;
    i32 error_a = (y_delta < x_delta ? x_delta : -y_delta) / 2;
    i32 error_b;
    for (;;) {
        mask[(y0 * WIDTH) + x0] = lightGray;
        if ((x0 == x1) && (y0 == y1)) {
            return;
        }
        error_b = error_a;
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
        pixel* p = pixels++;
        color* m = mask++;
        switch (*m) {
        case darkGray: {
            p->red = DARK_GRAY;
            p->green = DARK_GRAY;
            p->blue = DARK_GRAY;
            break;
        }
        case lightGray: {
            p->red = LIGHT_GRAY;
            p->green = LIGHT_GRAY;
            p->blue = LIGHT_GRAY;
            break;
        }
        }
    }
}

int main(void) {
    fileHandle* file = fopen(FILEPATH, "wb");
    if (file == NULL) {
        return EXIT_FAILURE;
    }
    memoryPool* memory = calloc(sizeof(memoryPool), 1);
    if (memory == NULL) {
        return EXIT_FAILURE;
    }
    bmpFile* bmp_buffer = &memory->bmp_buffer;
    set_bmp_header(&bmp_buffer->bmp_header);
    set_dib_header(&bmp_buffer->dib_header);
    pcgRng rng;
    rng.state = PCG_CONSTANT * get_microseconds();
    rng.increment = PCG_CONSTANT * get_microseconds();
    for (u8 i = 0; i < 16; ++i) {
        set_line(memory->mask,
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH),
                 (i32)pcg_32_bound(&rng, WIDTH));
    }
    set_pixels(memory->mask, bmp_buffer->pixels);
    u32 filesize = sizeof(bmpFile);
    if (fwrite(bmp_buffer, 1, filesize, file) != filesize) {
        return EXIT_FAILURE;
    }
    fclose(file);
    free(memory);
    return EXIT_SUCCESS;
}
