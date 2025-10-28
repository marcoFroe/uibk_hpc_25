#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include <stdbool.h>

// based on the heat stencil 2D implementation from the parallel programming ps
#define IND(y, x) ((y) * (N) + (x))
#define RESOLUTION_WIDTH 50
#define RESOLUTION_HEIGHT 50

void printTemperature(double* m, int N, int M) {
	const char* colors = " .-:=+*^X#%@";
	const int numColors = 12;

	// boundaries for temperature (for simplicity hard-coded)
	const double max = 273 + 30;
	const double min = 273 + 0;

	// set the 'render' resolution
	int W = RESOLUTION_WIDTH;
	int H = RESOLUTION_HEIGHT;

	// step size in each dimension
	int sW = N / W;
	int sH = M / H;

	// upper wall
	printf("\t");
	for(int u = 0; u < W + 2; u++) {
		printf("X");
	}
	printf("\n");
	// room
	for(int i = 0; i < H; i++) {
		// left wall
		printf("\tX");
		// actual room
		for(int j = 0; j < W; j++) {
			// get max temperature in this tile
			double max_t = 0;
			for(int x = sH * i; x < sH * i + sH; x++) {
				for(int y = sW * j; y < sW * j + sW; y++) {
					max_t = (max_t < m[IND(x, y)]) ? m[IND(x, y)] : max_t;
				}
			}
			double temp = max_t;

			// pick the 'color'
			int c = ((temp - min) / (max - min)) * numColors;
			c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

			// print the average temperature
			printf("%c", colors[c]);
		}
		// right wall
		printf("X\n");
	}
	// lower wall
	printf("\t");
	for(int l = 0; l < W + 2; l++) {
		printf("X");
	}
	printf("\n");
}


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

    int source[2];
    if(rank == 0) {    
        srand(time(NULL));
        source[0] = rand() % N; //y
        source[1] = rand() % N; //x
    }

    MPI_Bcast(source, 2, MPI_INT, 0, MPI_COMM_WORLD);
    int upper_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
	int lower_rank = (rank == num_ranks - 1) ? MPI_PROC_NULL : rank + 1;

    int relative_source_y = source[0] - (rank * sec_size) + 1;

    bool contains_source = false;

    if(relative_source_y >= 0 && relative_source_y < sec_size + 2) {
        current[IND(relative_source_y, source[1])] = 333;
        contains_source = true;
    }

    for(int t = 0; t < T; t++) {
        MPI_Sendrecv(current + N, N, MPI_DOUBLE, upper_rank, 42, current + N * (sec_size + 1), N, MPI_DOUBLE,
		             lower_rank, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		MPI_Sendrecv(current + sec_size * N, N, MPI_DOUBLE, lower_rank, 43, current, N,
		             MPI_DOUBLE, upper_rank, 43, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for(int i = 1; i < sec_size + 1; i++) {
            double here = current[IND(i, 0)];
            current[IND(i, 0)] = here + 0.2 * (273 + current[IND(i, 1)] + current[IND(i - 1, 0)] + current[IND(i + 1, 0)] - 4 * here);
            for(int j = 1; j < N - 1; j++) {
                double up = current[IND(i - 1, j)];
                double down = current[IND(i + 1, j)];
                double left = current[IND(i, j - 1)];
                double right = current[IND(i, j + 1)];
                here = current[IND(i, j)];


                next[IND(i, j)] = here + 0.2 * (up + down + left + right - (4 * here));
            }
            here = current[IND(i, N - 1)];
            current[IND(i, N - 1)] = here + 0.2 * (273 + current[IND(i, N - 2)] + current[IND(i - 1, N - 1)] + current[IND(i + 1, N - 1)] - 4 * here);
        }

        if(contains_source) {
            next[IND(relative_source_y, source[1])] = 333;
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
    
    int success = 1;

    for(int i = 1; i < sec_size + 1; i++) {
        for(int j = 0; j < N; j++) {
            double temp = current[IND(i, j)];
            if(273 <= temp && temp <= 273 + 60) continue;
            success = 0;
            break;
        }
    }

    printf("Verification of rank %d: %s\n", rank, (success) ? "OK" : "FAILED");

    MPI_Gather(current + N, N * sec_size, MPI_DOUBLE, aggr, N * sec_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    
    if(rank == 0) {
        printf("Final:");
        printTemperature(current, N, N);
        printf("\n");
    }

    MPI_Finalize();
    free(current);
    free(next);
    if (rank == 0) {
        free(aggr);
    }
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}