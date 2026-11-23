# Lab 7: Programming using Threads

Parallel matrix arithmetic (add, subtract, and dot product) with POSIX threads.

Two 20 x 20 matrices are filled with random values from 1 to 10. The work is split across **10 threads**, and each thread owns a contiguous block of rows (2 rows each for a 20 x 20 matrix). Each thread computes its rows of:

- **Sum**: `A[i][j] + B[i][j]`
- **Difference**: `A[i][j] - B[i][j]`
- **Product**: row `i` of A dotted with column `j` of B

Each thread gets its thread number in its own `malloc`'d memory, which the thread frees when it finishes. Because the threads write to rows that don't overlap, they need no locks. `main` joins all 10 threads before printing the results.

## Build and run

```sh
cd matrix
make
./matrix        # 20 x 20 (default)
./matrix 25     # optional: any size N; rows are spread evenly over the 10 threads
```
