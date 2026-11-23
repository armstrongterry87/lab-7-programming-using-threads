#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 10
#define DEFAULT_SIZE 20

int MAX;

// Matrices are stored as MAX x MAX blocks in row-major order.
// Access element (i, j) with M(mat, i, j).
#define M(mat, i, j) ((mat)[(i) * MAX + (j)])

int *matA;
int *matB;

int *matSumResult;
int *matDiffResult;
int *matProductResult;

void fillMatrix(int *matrix) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            M(matrix, i, j) = rand()%10+1;
        }
    }
}

void printMatrix(int *matrix) {
    for(int i = 0; i<MAX; i++) {
        for(int j = 0; j<MAX; j++) {
            printf("%5d", M(matrix, i, j));
        }
        printf("\n");
    }
    printf("\n");
}

// Each thread receives its thread number (0 .. NUM_THREADS-1) and owns a
// contiguous block of rows. For a 20 x 20 matrix with 10 threads, every
// thread computes 2 rows. Any remainder rows are spread over the first threads.
static void rowRange(int t, int *start, int *end) {
    int base = MAX / NUM_THREADS;
    int extra = MAX % NUM_THREADS;
    *start = t * base + (t < extra ? t : extra);
    *end = *start + base + (t < extra ? 1 : 0);
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matSumResult at the coordinates to the sum of the
// values at the coordinates of matA and matB.
void* computeSum(void* args) { // pass in the number of the ith thread
    int t = *(int *)args;
    int start, end;
    rowRange(t, &start, &end);
    for(int i = start; i < end; i++) {
        for(int j = 0; j < MAX; j++) {
            M(matSumResult, i, j) = M(matA, i, j) + M(matB, i, j);
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matDiffResult at the coordinates to the difference of the
// values at the coordinates of matA and matB.
void* computeDiff(void* args) { // pass in the number of the ith thread
    int t = *(int *)args;
    int start, end;
    rowRange(t, &start, &end);
    for(int i = start; i < end; i++) {
        for(int j = 0; j < MAX; j++) {
            M(matDiffResult, i, j) = M(matA, i, j) - M(matB, i, j);
        }
    }
    return NULL;
}

// Fetches the appropriate coordinates from the argument, and sets
// the cell of matProductResult at the coordinates to the inner product
// of matA and matB (row i of A dotted with column j of B).
void* computeProduct(void* args) { // pass in the number of the ith thread
    int t = *(int *)args;
    int start, end;
    rowRange(t, &start, &end);
    for(int i = start; i < end; i++) {
        for(int j = 0; j < MAX; j++) {
            int dot = 0;
            for(int k = 0; k < MAX; k++) {
                dot += M(matA, i, k) * M(matB, k, j);
            }
            M(matProductResult, i, j) = dot;
        }
    }
    return NULL;
}

// Thread entry point: each of the 10 threads computes its share of the
// sum, difference and product, then frees the memory holding its number.
void* worker(void* args) {
    computeSum(args);
    computeDiff(args);
    computeProduct(args);
    free(args);
    return NULL;
}

// Spawn a thread to fill each cell in each result matrix.
// How many threads will you spawn? 10 in total; each one handles
// MAX/10 rows of all three result matrices.
int main(int argc, char *argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX
    MAX = DEFAULT_SIZE;
    if (argc > 1) {
        MAX = atoi(argv[1]);
        if (MAX <= 0) {
            fprintf(stderr, "Usage: %s [matrix size > 0]\n", argv[0]);
            return 1;
        }
    }

    size_t cells = (size_t)MAX * MAX;
    matA = malloc(cells * sizeof(int));
    matB = malloc(cells * sizeof(int));
    matSumResult = malloc(cells * sizeof(int));
    matDiffResult = malloc(cells * sizeof(int));
    matProductResult = malloc(cells * sizeof(int));
    if (!matA || !matB || !matSumResult || !matDiffResult || !matProductResult) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads.
    pthread_t threads[NUM_THREADS];

    // 4. Create the threads. Each thread gets a malloc'd copy of its
    //    thread number, which it uses to find its rows and frees when done.
    for (int t = 0; t < NUM_THREADS; t++) {
        int *id = malloc(sizeof(int));
        if (!id) {
            fprintf(stderr, "Out of memory\n");
            return 1;
        }
        *id = t;
        if (pthread_create(&threads[t], NULL, worker, id) != 0) {
            fprintf(stderr, "Error creating thread %d\n", t);
            free(id);
            return 1;
        }
    }

    // 5. Wait for all threads to finish.
    for (int t = 0; t < NUM_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);

    free(matA);
    free(matB);
    free(matSumResult);
    free(matDiffResult);
    free(matProductResult);
    return 0;
}
