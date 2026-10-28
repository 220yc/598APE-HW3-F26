
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static int N;
static int *storage;
static int *row_start;
static int *row_end;

static void die(const char *s) {
    fprintf(stderr, "%s\n", s);
    exit(EXIT_FAILURE);
}

static void render(const char *filename) {
    size_t count = (size_t)N * N;

    if (count > SIZE_MAX / 3)
        die("Image size overflow");

    unsigned char *pixels = malloc(count * 3);

    if (!pixels)
        die("malloc failed for PNG pixel buffer");

    for (size_t k = 0; k < count; ++k) {
        unsigned char r = 0, g = 0, b = 0;

        switch (storage[k]) {
            case 0:
                r = 15; g = 15; b = 25;
                break;
            case 1:
                r = 0; g = 210; b = 255;
                break;
            case 2:
                r = 230; g = 30; b = 130;
                break;
            case 3:
                r = 255; g = 200; b = 0;
                break;
        }

        pixels[3*k] = r;
        pixels[3*k+1] = g;
        pixels[3*k+2] = b;
    }

    if (!stbi_write_png(
            filename, N, N, 3, pixels, N*3)) {
        free(pixels);
        die("Failed to save PNG");
    }

    printf(
        "Successfully saved sandpile identity to: %s\n",
        filename
    );

    free(pixels);
}

static void prepare_geometry(void) {
    int center = N / 2;
    int64_t radius2 = (int64_t)center * center;

    for (int i = 0; i < N; ++i) {
        int bound = i > N-1-i ? i : N-1-i;

        int64_t di = (int64_t)i - center;
        int64_t remaining = radius2 - di*di;

        if (remaining < 0) {
            row_start[i] = 1;
            row_end[i] = 0;
            continue;
        }

        int limit = (int)sqrt((double)remaining);

        while ((int64_t)(limit+1)*(limit+1)
               <= remaining)
            ++limit;

        while ((int64_t)limit*limit > remaining)
            --limit;

        int start = center - limit;
        int end = center + limit;

        if (start < 0)
            start = 0;

        if (end > bound)
            end = bound;

        row_start[i] = start;
        row_end[i] = end;
    }
}

static void stabilize(void) {
    int changed;

    do {
        changed = 0;

        for (int i = 0; i < N; ++i) {
            int start = row_start[i];
            int end = row_end[i];

            int *row = storage + (size_t)i*N;
            int *up = i > 0 ? row-N : NULL;
            int *down = i+1 < N ? row+N : NULL;

            for (int j = start; j <= end; ++j) {
                int value = row[j];

                if (value < 4)
                    continue;

                int q = value / 4;

                row[j] = value - 4*q;

                if (up)
                    up[j] += q;

                if (j > 0)
                    row[j-1] += q;

                if (down)
                    down[j] += q;

                if (j+1 < N)
                    row[j+1] += q;

                changed = 1;
            }
        }
    } while (changed);
}

static void find_identity(void) {
    size_t count = (size_t)N*N;

    for (size_t k = 0; k < count; ++k)
        storage[k] = 6;

    stabilize();

    for (size_t k = 0; k < count; ++k)
        storage[k] = 6 - storage[k];

    stabilize();
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s N\n", argv[0]);
        return EXIT_FAILURE;
    }

    errno = 0;
    char *end = NULL;

    uintmax_t parsed =
        strtoumax(argv[1], &end, 10);

    if (errno ||
        argv[1][0] < '0' ||
        argv[1][0] > '9' ||
        *end ||
        parsed == 0 ||
        parsed > INT_MAX)
        die("grid size must be a positive int");

    N = (int)parsed;

    if ((size_t)N > SIZE_MAX/(size_t)N ||
        (size_t)N*N > SIZE_MAX/sizeof(int) ||
        (size_t)N > SIZE_MAX/3)
        die("Grid size too large");

    storage = malloc((size_t)N*N*sizeof(int));
    row_start = malloc((size_t)N*sizeof(int));
    row_end = malloc((size_t)N*sizeof(int));

    if (!storage || !row_start || !row_end)
        die("Memory allocation failed");

    prepare_geometry();
    find_identity();
    render("identity.png");

    free(row_end);
    free(row_start);
    free(storage);

    return EXIT_SUCCESS;
}
