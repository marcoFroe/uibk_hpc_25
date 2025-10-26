#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <mpi.h>

// based on the heat stencil 2D implementation from the parallel programming ps
#define IND(y, x) ((y) * (N) + (x))

int failed_malloc() {
    fprintf(stderr, "Memory allocation error\n");
    MPI_Finalize();
    return EXIT_FAILURE;
}

int main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: ./seq <n> <t>\n");
        return EXIT_FAILURE;
    }
    int N = atoi(argv[1]);
    if(N < 2) {
        fprintf(stderr, "N needs to be greater than 1\n");
        return EXIT_FAILURE;
    }

    int T = atoi(argv[2]);

    MPI_Init(&argc, &argv);
	int num_ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if(N % num_ranks != 0) {
        if(rank == 0) {
            perror("Size must be divisible by number of MPI ranks!\n");
        }
    MPI_Finalize();
    return EXIT_FAILURE;
    }

    int sec_size = N / num_ranks;

    double* current = malloc(sizeof(double) * (sec_size + 2) * N);
    if(current == NULL) {
        failed_malloc();
    }

    double* next = malloc(sizeof(double) * (sec_size + 2) * N);
    if(next == NULL) {
        free(current);
        failed_malloc();
    }

    for(int i = 0; i < sec_size + 2; i++) {
        for(int j = 0; j < N; j++) {
            current[IND(i, j)] = 273;  
        }
    }

    int[2] source;
    if(rank == 0) {    
        srand(time(NULL));
        source[0] = rand() % N; //y
        source[1] = rand() % N; //x
    }

    MPI_Bcast(source, 2, MPI_INT, 0, MPI_COMM_WORLD);
    int upper_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
	int lower_rank = (rank == num_ranks - 1) ? MPI_PROC_NULL : rank + 1;

    int relative_source_y = source[0] - (rank * sec_size);

    if(relative_source_y >= 0 && relative_source_y < sec_size) {
        current[IND(relative_source_y, source[1])] = 333;
        next[IND(relative_source_y, source[1])] = 333;
    }

    for(int t = 0; t < T; t++) {
        MPI_Sendrecv(current + 1, N, MPI_DOUBLE, upper_rank, 42, current, N + sec_size + 1, MPI_DOUBLE,
		             lower_rank, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		MPI_Sendrecv(current + sec_size, N, MPI_DOUBLE, lower_rank, 43, current, N,
		             MPI_DOUBLE, upper_rank, 43, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i = 1; i < N - 1; i++) {
            for(int j = 0; j < N; j++) {
                if(i == relative_source_y && j == source[1]) {
                    continue;
                }

                double up = current[IND(i - 1, j)];
                double down = current[IND(i + 1, j)];
                double left = (j > 0) ? current[IND(i, j - 1)] : current[IND(i, j)];
                double right = (j < N - 1) ? current[IND(i, j + 1)] : current[IND(i, j)];
                double here = current[IND(i, j)];


                next[IND(i, j)] = here + 0.2 * (up + down + left + right + (-4 * here));
            }
        }
        
        double* temp = current;
        current = next;
        next = temp;
    }
    double* aggr;
    if(rank == 0) {
        aggr = malloc(sizeof(double) * N * N);
        if(aggr == NULL) {
            free(current);
            free(next);
            failed_malloc();
        }
    }

    MPI_Gather(current + 1, N * sec_size, MPI_DOUBLE, aggr, N * sec_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if(rank == 0) {
        int success = 1;
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                double temp = aggr[IND(i, j)];
                if(273 <= temp && temp <= 273 + 60) continue;
                success = 0;
                break;
            }
        }

        printf("Verification: %s\n", (success) ? "OK" : "FAILED");
    }

    MPI_Finalize();
    free(current);
    free(next);
    free(recv);
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}