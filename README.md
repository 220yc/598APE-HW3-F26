# CS598APE HW3

This repository contains code for homework 3 of CS598APE. It is relatively
simple in comparison to HW1 and HW2 to ensure you have enough time to work on
the course project. 

`sandpile.c` contains the implementation of finding the identity element of a 2D
abelian sandpile.

![Identity sandpile on a 1001x1001 pacman grid](identity-ref.png)

*Identity sandpile on a 1001x1001 pacman grid*


To compile the program, run:

```bash
make all
```

To clean existing build artifacts, run:

```bash
make clean
```

Usage (after compiling):

```bash
./sandpile.exe <N> 
```

where `N` is the grid size

Consider speeding up the generation of this 1001x1001 identity sandpile (which
takes approximately ~3m 40s on my local laptop under the default setup)

```bash
./sandpile.exe 1001
```

For an optional, ungraded challenge, see how large a sandpile you can compute
on the course VMs and report the grid size and runtime. There is no prescribed
upper limit on `N` for this challenge.


---

# Optimization Artifact — Abelian Sandpile

## Overview

This artifact accompanies the CS598APE HW3 mini-paper. It documents
three optimization iterations of the Abelian sandpile identity
computation, including an unsuccessful queue-based implementation.

The final optimized implementation is `sandpile.c` in the repository
root. It corresponds to version 3 (v3) in the report.

For N=1001, the final version achieved a 2.35x speedup over the
original baseline, reducing wall-clock time from 610.70 seconds
to 259.80 seconds while producing byte-identical PNG output.

## Requirements

- Linux or another Unix-like environment
- GCC with C language support
- GNU Make
- Standard C library and math library (`-lm`)
- GNU time (`/usr/bin/time`) for benchmark measurements
- `cmp` for byte-for-byte correctness verification
- No GPU or specialized hardware required

The project uses GCC optimization level `-O3` with debugging
symbols and warnings treated as errors.

## Repository Structure

- `sandpile.c`: final optimized implementation (v3)
- `Makefile`: builds the final executable
- `baseline/`: original baseline implementation
- `experiments/v3/`: final optimized source and experiment files
- `results/`: saved benchmark measurements and correctness outputs
- `README_original.md`: original assignment README

Earlier experimental versions are retained in the repository's
experiment and results files where applicable.

## Build

From the repository root:

```bash
make clean
make all

```

## Run

After building, run:

    ./sandpile.exe 31

For the full workload:

    ./sandpile.exe 1001

The program writes its output to `identity.png`.

## Correctness Verification

The final optimized implementation was checked against the original baseline using byte-for-byte PNG comparisons.

Run the following commands from the repository root:

    ./sandpile.exe 31
    cmp -s identity.png results/baseline_31.png && echo "PASS: N=31"

    ./sandpile.exe 201
    cmp -s identity.png results/baseline_201.png && echo "PASS: N=201"

    ./sandpile.exe 1001
    cmp -s identity.png results/baseline_identity.png && echo "PASS: N=1001"

All three sizes passed in the recorded experiments.

## Benchmark Reproduction

The experiments were performed on the UIUC course VM `fa26-cs598a-049`, using GCC optimization level `-O3`.

Build the final implementation:

    make all

Measure its performance:

    /usr/bin/time -v ./sandpile.exe 1001

Build and benchmark the original baseline:

    gcc baseline/sandpile_original.c -o baseline/sandpile_original.exe -O3 -lm -g -Werror
    /usr/bin/time -v ./baseline/sandpile_original.exe 1001

The main reported measurements are wall-clock time and maximum resident set size. Results may vary depending on hardware and machine load.

## Experimental Results

| Version | Wall Time (s) | Speedup | Peak RSS (KB) |
|---|---:|---:|---:|
| Baseline | 610.70 | 1.00x | 12472 |
| v1 | 1352.80 | 0.45x | 22316 |
| v2 | 387.77 | 1.57x | 12600 |
| v3 | 259.80 | 2.35x | 12628 |

The final version achieved a 57.5% runtime reduction relative to the baseline.

## Optimization History

### Version 1: Active Queue and Batched Toppling

An active-cell queue was introduced to avoid unnecessary grid scans, together with batched toppling. However, the implementation was slower than the baseline, potentially due to queue-management overhead and irregular memory accesses.

### Version 2: Sequential Scanning and Contiguous Storage

The queue was removed. Batched toppling was retained, and contiguous memory storage with sequential scanning improved runtime to 387.77 seconds.

### Version 3: Geometry-Aware Scanning and Direct Pointers

Precomputed valid column intervals avoided repeated geometry checks and unnecessary cell visits. Direct pointer-based neighbor access further simplified the stabilization loop. The combined changes reduced runtime to 259.80 seconds.

## Profiling and Limitations

GNU `gprof` was used to profile v1 at N=101. The sampled execution time was attributed to `stabilize()`.

Hardware performance counters were unavailable because the course VM restricted `perf` access.

Benchmark measurements were collected as single runs. The individual changes introduced in v3 were not separately benchmarked.

## Reproducibility Notes

The final implementation requires no GPU or specialized hardware. The original toppling rule and identity construction are preserved. Saved experimental measurements and source versions are retained in the repository.
