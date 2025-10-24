#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef double value_t;

#define RESOLUTION 120
#define GOOD_TAG 42

// -- vector utilities --

typedef value_t* Vector;

Vector createVector(int N);

void releaseVector(Vector m);

void printTemperature(Vector m, int N);

// -- simulation code ---

int main(int argc, char** argv) {
	// Init MPI stuff
	MPI_Init(&argc, &argv);
	int num_ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// 'parsing' optional input parameter = problem size
	int N = 2000;
	if(argc > 1) {
		N = atoi(argv[1]);
	}

	if(N % num_ranks != 0) {
		if(rank == 0) {
			perror("Size must be divisible by number of MPI ranks!\n");
		}
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}

	int T = N * 500;

	// ---------- setup ----------

	// heat source position at random position to make it more interesting
	int source_x;
	if(rank == 0) {
		srand(time(NULL));
		source_x = rand() % N;
	}

	MPI_Bcast(&source_x, 1, MPI_INT, 0, MPI_COMM_WORLD);
	// create global buffer
	Vector A = createVector(N);
	// set up initial conditions in A
	for(int i = 0; i < N; i++) {
		A[i] = 273; // temperature is 0° C everywhere (273 K)
	}

	A[source_x] = 273 + 60;

	if(rank == 0) {
		printf("Computing heat-distribution for room size N=%d for T=%d timesteps\n", N, T);
		printf("Initial:\t");
		printTemperature(A, N);
		printf("\n");
	}

	// Create local subdomain
	int sec_size = N / num_ranks;
	Vector SubSec = createVector(sec_size);
	for (int i = 0; i < sec_size; i++) {
    	SubSec[i] = 273; // Default temperature
	}

	// setup exchange edges
	value_t left = 273;
	value_t right = 273;

	// ---------- compute ----------

	// create a second buffer for the computation
	Vector B = createVector(sec_size);
	// Edge Synchronisation -> MPI_PROC_NULL does not change the buffers if used in Send/Recv
	int left_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
	int right_rank = (rank == num_ranks - 1) ? MPI_PROC_NULL : rank + 1;

    // Heat source calculations
	int global_source_start = rank * sec_size;
	int global_source_end = global_source_start + sec_size;

    // Requests
    int num_rq=4;
    MPI_Request rq_array[num_rq];

	// for each time step ..
	for(int t = 0; t < T; t++) {
        // Heat source stays constant
		if (source_x >= global_source_start && source_x < global_source_end) {
    		int local_source_pos = source_x - global_source_start;
    		SubSec[local_source_pos] = 273 + 60; 
		}

		// exchange boundaries -> non blocking
        MPI_Isend(&SubSec[0], 1, MPI_DOUBLE, left_rank, GOOD_TAG, MPI_COMM_WORLD, rq_array);
        MPI_Isend(&SubSec[sec_size-1], 1, MPI_DOUBLE, right_rank, GOOD_TAG,MPI_COMM_WORLD, rq_array+2);

        MPI_Irecv(&left, 1, MPI_DOUBLE, left_rank, GOOD_TAG, MPI_COMM_WORLD,rq_array+1);
        MPI_Irecv(&right, 1, MPI_DOUBLE, right_rank, GOOD_TAG, MPI_COMM_WORLD, rq_array+3);
		
        // .. we propagate the temperature inner cells
		for(long long i = 1; i < sec_size-1; i++) {
			// get temperature at current position
			value_t tc = SubSec[i];

			// get temperatures of adjacent cells
			value_t tl = SubSec[i - 1];
			value_t tr =  SubSec[i + 1];

			// compute new temperature at current position
			B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
		}

        // Calculate edges
        MPI_Waitall(num_rq, rq_array, MPI_STATUS_IGNORE);
        B[0] = SubSec[0]+0.2 *(left+SubSec[1]+(-2*SubSec[0]));
        B[sec_size-1] = SubSec[sec_size-1]+0.2 *(SubSec[sec_size-2]+right+(-2*SubSec[sec_size-1]));

		// swap matrices (just pointers, not content)
		Vector H = SubSec;
		SubSec = B;
		B = H;

		// show intermediate step
		if(!(t % 10000)) {
			MPI_Gather(SubSec, sec_size, MPI_DOUBLE, A, sec_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			if(rank == 0) {
				printf("Step t=%d:\t", t);
				printTemperature(A, N);
				printf("\n");
			}
		}
	}
	MPI_Gather(SubSec, sec_size, MPI_DOUBLE, A, sec_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);
	releaseVector(B);
	releaseVector(SubSec);

	// ---------- check ----------
	int success = 1;
	if(rank == 0) {
		printf("Final:\t\t");
		printTemperature(A, N);
		printf("\n");

		for(long long i = 0; i < N; i++) {
			value_t temp = A[i];
			if(273 <= temp && temp <= 273 + 60) continue;
			success = 0;
			break;
		}

		printf("Verification: %s\n", (success) ? "OK" : "FAILED");
	}
	// ---------- cleanup ----------

	releaseVector(A);

	MPI_Finalize();
	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

Vector createVector(int N) {
	// create data and index vector
	return malloc(sizeof(value_t) * N);
}

void releaseVector(Vector m) {
	free(m);
}

void printTemperature(Vector m, int N) {
	const char* colors = " .-:=+*^X#%@";
	const int numColors = 12;

	// boundaries for temperature (for simplicity hard-coded)
	const value_t max = 273 + 30;
	const value_t min = 273 + 0;

	// set the 'render' resolution
	int W = RESOLUTION;

	// step size in each dimension
	int sW = N / W;

	// room
	// left wall
	printf("X");
	// actual room
	for(int i = 0; i < W; i++) {
		// get max temperature in this tile
		value_t max_t = 0;
		for(int x = sW * i; x < sW * i + sW; x++) {
			max_t = (max_t < m[x]) ? m[x] : max_t;
		}
		value_t temp = max_t;

		// pick the 'color'
		int c = ((temp - min) / (max - min)) * numColors;
		c = (c >= numColors) ? numColors - 1 : ((c < 0) ? 0 : c);

		// print the average temperature
		printf("%c", colors[c]);
	}
	// right wall
	printf("X");
}
