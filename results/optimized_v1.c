
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

static int N;
static int *grid;
static unsigned char *active;
static unsigned char *queued;
static size_t *queue;
static size_t capacity, head, tail, count;

static void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

// Add an unstable cell to the work queue.
static void enqueue(size_t k) {
    if (queued[k]) return;
    if (count == capacity) die("Queue overflow");

    queue[tail] = k;
    tail = (tail + 1 == capacity) ? 0 : tail + 1;
    ++count;
    queued[k] = 1;
}

// Add grains and schedule active unstable cells.
static void add_grains(size_t k, int grains) {
    grid[k] += grains;

    if (active[k] && grid[k] >= 4) {
        enqueue(k);
    }
}

// Stabilize using batched toppling and a work queue.
static void stabilize(void) {
    head = tail = count = 0;
    memset(queued, 0, capacity);

    for (size_t k = 0; k < capacity; ++k) {
        if (active[k] && grid[k] >= 4) {
            enqueue(k);
        }
    }

    while (count) {
        size_t k = queue[head];
        head = (head + 1 == capacity) ? 0 : head + 1;
        --count;
        queued[k] = 0;

        if (grid[k] < 4) continue;

        int q = grid[k] / 4;
        grid[k] -= 4 * q;

        int i = (int)(k / (size_t)N);
        int j = (int)(k % (size_t)N);

        if (i > 0)
            add_grains(k - (size_t)N, q);

        if (j > 0)
            add_grains(k - 1, q);

        if (i + 1 < N)
            add_grains(k + (size_t)N, q);

        if (j + 1 < N)
            add_grains(k + 1, q);
    }
}

// Compute the sandpile identity.
static void find_identity(void) {
    for (size_t k = 0; k < capacity; ++k) {
        grid[k] = 6;
    }

    stabilize();

    for (size_t k = 0; k < capacity; ++k) {
        grid[k] = 6 - grid[k];
    }

    stabilize();
}

// Render the grid with the original color mapping.
static void render(const char *filename) {
    if (capacity > SIZE_MAX / 3)
        die("Image too large");

    unsigned char *pixels = malloc(capacity * 3);

    if (!pixels)
        die("Cannot allocate image buffer");

    static const unsigned char colors[4][3] = {
        {15, 15, 25},
        {0, 210, 255},
        {230, 30, 130},
        {255, 200, 0}
    };

    for (size_t k = 0; k < capacity; ++k) {
        int v = grid[k];

        if (v >= 0 && v <= 3) {
            memcpy(pixels + 3 * k, colors[v], 3);
        } else {
            memset(pixels + 3 * k, 0, 3);
        }
    }

    if (!stbi_write_png(
            filename, N, N, 3, pixels, N * 3)) {
        die("Failed to write PNG");
    }

    printf(
        "Successfully saved sandpile identity to: %s\n",
        filename
    );

    free(pixels);
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
        (size_t)N * (size_t)N >
            SIZE_MAX / sizeof(int)) {
        die("Grid too large");
    }

    capacity = (size_t)N * (size_t)N;

    grid = malloc(capacity * sizeof(int));
    active = calloc(capacity, 1);
    queued = calloc(capacity, 1);
    queue = malloc(capacity * sizeof(size_t));

    if (!grid || !active || !queued || !queue)
        die("Allocation failed");

    // Preserve the original Pac-Man geometry.
    int center = N / 2;
    int64_t radius2 = (int64_t)center * center;

    for (int i = 0; i < N; ++i) {
        int bound = (i > N - 1 - i)
            ? i : N - 1 - i;

        for (int j = 0; j < N && j <= bound; ++j) {
            int64_t di = (int64_t)i - center;
            int64_t dj = (int64_t)j - center;

            if (di * di + dj * dj <= radius2) {
                active[
                    (size_t)i * (size_t)N +
                    (size_t)j
                ] = 1;
            }
        }
    }

    find_identity();
    render("identity.png");

    free(queue);
    free(queued);
    free(active);
    free(grid);

    return EXIT_SUCCESS;
}
