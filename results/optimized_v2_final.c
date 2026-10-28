
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
static int **grid;
static int *storage;
static int *row_end;

static void die(const char *s) {
    fprintf(stderr, "%s\n", s);
    exit(EXIT_FAILURE);
}

// Preserve the original PNG output.
static void render(const char *filename) {
    size_t pixels_count = (size_t)N * N;

    if (pixels_count > SIZE_MAX / 3)
        die("Image size overflow");

    unsigned char *pixels = malloc(pixels_count * 3);

    if (!pixels)
        die("malloc failed for PNG pixel buffer");

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;

            switch (grid[i][j]) {
                case 0:
                    r = 15;
                    g = 15;
                    b = 25;
                    break;
                case 1:
                    r = 0;
                    g = 210;
                    b = 255;
                    break;
                case 2:
                    r = 230;
                    g = 30;
                    b = 130;
                    break;
                case 3:
                    r = 255;
                    g = 200;
                    b = 0;
                    break;
                default:
                    r = 0;
                    g = 0;
                    b = 0;
                    break;
            }

            size_t k = ((size_t)i * N + j) * 3;

            pixels[k] = r;
            pixels[k + 1] = g;
            pixels[k + 2] = b;
        }
    }

    if (stbi_write_png(
            filename, N, N, 3, pixels, N * 3)) {
        printf(
            "Successfully saved sandpile identity to: %s\n",
            filename
        );
    } else {
        free(pixels);
        die("Failed to save PNG");
    }

    free(pixels);
}

// Precompute valid Pac-Man row boundaries.
static void prepare_geometry(void) {
    int center = N / 2;
    int64_t radius2 = (int64_t)center * center;

    for (int i = 0; i < N; ++i) {
        int bound = i > N - 1 - i
            ? i : N - 1 - i;

        if (bound >= N)
            bound = N - 1;

        int64_t di = (int64_t)i - center;
        int64_t remaining = radius2 - di * di;

        if (remaining < 0) {
            row_end[i] = -1;
            continue;
        }

        int limit = (int)sqrt((double)remaining);

        // Correct possible floating-point rounding.
        while ((int64_t)(limit + 1) *
                   (limit + 1) <= remaining) {
            ++limit;
        }

        while ((int64_t)limit * limit > remaining) {
            --limit;
        }

        int end = center + limit;

        if (end > bound)
            end = bound;

        row_end[i] = end;
    }
}

// Batched toppling with sequential scanning.
static void stabilize(void) {
    int changed;

    do {
        changed = 0;

        for (int i = 0; i < N; ++i) {
            int end = row_end[i];

            if (end < 0)
                continue;

            int64_t di = (int64_t)i - N / 2;
            int64_t radius2 =
                (int64_t)(N / 2) * (N / 2);

            for (int j = 0; j <= end; ++j) {
                int64_t dj = (int64_t)j - N / 2;

                if (di * di + dj * dj > radius2)
                    continue;

                int value = grid[i][j];

                if (value < 4)
                    continue;

                int q = value / 4;

                grid[i][j] = value - 4 * q;

                if (i > 0)
                    grid[i - 1][j] += q;

                if (j > 0)
                    grid[i][j - 1] += q;

                if (i + 1 < N)
                    grid[i + 1][j] += q;

                if (j + 1 < N)
                    grid[i][j + 1] += q;

                changed = 1;
            }
        }
    } while (changed);
}

static void find_identity(void) {
    for (size_t k = 0; k < (size_t)N * N; ++k)
        storage[k] = 6;

    stabilize();

    for (size_t k = 0; k < (size_t)N * N; ++k)
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

    uintmax_t parsed = strtoumax(argv[1], &end, 10);

    if (errno ||
        argv[1][0] < '0' ||
        argv[1][0] > '9' ||
        *end ||
        parsed == 0 ||
        parsed > INT_MAX) {
        die("grid size must be a positive int");
    }

    N = (int)parsed;

    if ((size_t)N > SIZE_MAX / (size_t)N ||
        (size_t)N * N > SIZE_MAX / sizeof(int)) {
        die("Grid size too large");
    }

    grid = malloc((size_t)N * sizeof(int *));
    storage = malloc((size_t)N * N * sizeof(int));
    row_end = malloc((size_t)N * sizeof(int));

    if (!grid || !storage || !row_end)
        die("Memory allocation failed");

    for (int i = 0; i < N; ++i)
        grid[i] = storage + (size_t)i * N;

    prepare_geometry();
    find_identity();
    render("identity.png");

    free(row_end);
    free(storage);
    free(grid);

    return EXIT_SUCCESS;
}
