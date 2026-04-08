# Word Frequency Analysis — Naive, Multiprocessing & Multithreading

A C project developed as part of a Operating Systems course. It analyzes the [enwik8](https://mattmahoney.net/dc/enwik8.zip) dataset to find the top 10 most frequent words using three different approaches: naive (sequential), multiprocessing, and multithreading. Execution times are compared across different numbers of processes/threads to evaluate the impact of parallelism.

## Approaches

### Naive (`naive.c`)
Single-threaded sequential processing. Reads the entire file, counts word frequencies, sorts using QuickSort, and prints the top 10. Serves as the performance baseline.

### Multiprocessing (`multiprocessing.c`)
Splits the file into chunks and forks child processes using `fork()`. Each process counts words in its chunk independently, then merges results into shared memory allocated with `mmap()`. Parent waits for all children using `wait()`.

### Multithreading (`multithreading.c`)
Splits the file into chunks and spawns threads using `pthread_create()`. Each thread counts words locally then merges into a shared global array protected by a `pthread_mutex_t` mutex.

## Performance Results

| Approach | 2 | 4 | 6 | 8 |
|---|---|---|---|---|
| Naive | 622.9s | — | — | — |
| Multiprocessing | 455.1s | 376.5s | 271.6s | 267.5s |
| Multithreading | — | 504.2s | 469.9s | 461.9s |

Tested on: Intel Core i7-1165G7 (4 cores, 2.8 GHz), 7.7 GB RAM, SSD, Windows with WSL (Windows Subsystem for Linux).

## Key Findings

- Multiprocessing outperforms multithreading due to lower synchronization overhead
- Best multiprocessing speedup at 8 processes: ~2.33x over naive
- Amdahl's Law theoretical max speedup (4 cores): ~2.11x for multiprocessing, ~2.29x for multithreading
- Performance gains diminish beyond 4–6 processes/threads due to contention and overhead

## How to Compile and Run

```bash
# Naive
gcc naive.c -o naive
./naive

# Multiprocessing
gcc multiprocessing.c -o multiprocessing
./multiprocessing

# Multithreading
gcc multithreading.c -o multithreading -lpthread
./multithreading
```

Make sure `text8.txt` (the enwik8 dataset renamed) is in the same directory as the executables.

## Repository Structure

```
├── naive.c
├── multiprocessing.c
├── multithreading.c
└── README.md
```
