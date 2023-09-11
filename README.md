# MPI & OpenMP Hybrid Project

This project showcases the implementation of a hybrid approach combining MPI (Message Passing Interface) and OpenMP (Open Multi-Processing) to parallelize computation and communication in a distributed-memory environment.

## Description

The main program takes an input file of double numbers, splits the data between two MPI processes, and then further parallelizes computations within these processes using OpenMP. The goal is to calculate a specific mathematical function (`calculateBElement`) on each of the numbers and find the minimum value.

## Requirements

- MPI installed (e.g., `mpich`, `openmpi`).
- OpenMP support in your compiler (usually default in modern compilers like GCC).
- C compiler with support for MPI and OpenMP.

## Input Data
Place your input.dat file in the ../resources/ directory. The file should contain:

On the first line, an integer N representing the total number of double numbers in the file.
On the subsequent lines, N double numbers, one per line.

## Details
MPI is used to distribute the data from the file between two processes: one "master" and one "slave".
OpenMP is then used within each process to parallelize the computation across threads.
The master process reads the input data, splits it, and sends a portion to the slave process. Both then compute in parallel.
The results (minimum value of the calculateBElement function) from both processes are collected in the master process, which then prints the final result and the average execution time over the given number of iterations.
