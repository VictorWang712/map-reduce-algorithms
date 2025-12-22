# map-reduce-algorithms

## Introduction

MapReduce is a programming model designed for processing large-scale data using parallel and distributed computation. It consists of two core phases:

- **Map phase**: Input data is split and processed in parallel, producing intermediate key-value pairs.
- **Reduce phase**: Intermediate results are aggregated to produce the final output.

This project implements a word count application using both a serial algorithm and a parallel MapReduce-style algorithm written in C.

## Implementation

### Serial Version

- Reads the entire file sequentially
- Uses a hash table to count word occurrences
- Sorts results by descending frequency and lexicographical order

### Parallel MapReduce Version

- Splits the input file into chunks
- Multiple threads perform the Map phase
- Each thread maintains a local hash table
- Results are merged in the Reduce phase
- Final output is sorted

## Usage

### Compile and Run Benchmark

```bash
cd benchmark
python evaluator.py
```

### Output

- Word count results are saved in `data/output`

- Performance logs are saved in `benchmark/log`

## Performance Analysis

Performance depends on:

- Hardware (CPU cores, cache)

- Input size

- Number of Map threads

- Overhead of thread creation and synchronization

- Parallel execution significantly reduces runtime for large input files compared to the serial approach.
