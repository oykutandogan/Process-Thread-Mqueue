# FindTopK - Process, Message Queue & Thread in C

This project demonstrates three different methods (**process**, **POSIX message queue**, and **thread**) to generate `k` random numbers in multiple files, find the maximum in each file, and write results to an output file.

---

## Features
- Generates `k` random numbers per file.
- Finds the maximum number in each file.
- Compares performance of **fork**, **message queues**, and **threads**.
- Displays execution time.

---

## Usage
```bash
./findtopk <k> <N> <infile1> ... <infileN> <outfile>
```

## Compilation
```bash
gcc -o findtopk findtopk.c
gcc -o findtopk_mqueue findtopk_mqueue.c -lrt
gcc -o findtopk_thread findtopk_thread.c -lpthread
```

## Requirements
GCC Compiler

POSIX Threads (-lpthread)

POSIX Message Queues (-lrt)
