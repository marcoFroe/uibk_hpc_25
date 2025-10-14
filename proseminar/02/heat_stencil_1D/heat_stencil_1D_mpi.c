#include <math.h>
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
		return EXIT_FAILURE;
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

	// Distribute Data
	int sec_size = N / num_ranks;
	Vector SubSec = createVector(sec_size);
	MPI_Scatter(A, sec_size, MPI_DOUBLE, SubSec, sec_size, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	// setup exchange edges
	value_t left = 273;
	value_t right = 273;

	// ---------- compute ----------

	// create a second buffer for the computation
	Vector B = createVector(sec_size);
	// Edge Synchronisation -> MPI_PROC_NULL does not change the buffers if used in Send/Recv
	int left_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
	int right_rank = (rank == num_ranks - 1) ? MPI_PROC_NULL : rank + 1;

	int relative_source_pos = source_x - (rank * sec_size);

	value_t left_recv = 273;
	value_t right_recv = 273;

	// for each time step ..
	for(int t = 0; t < T; t++) {
		// exchange boundaries
		MPI_Sendrecv(&SubSec[0], 1, MPI_DOUBLE, left_rank, GOOD_TAG, &right_recv, 1, MPI_DOUBLE,
		             right_rank, GOOD_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		MPI_Sendrecv(&SubSec[sec_size - 1], 1, MPI_DOUBLE, right_rank, GOOD_TAG + 1, &left_recv, 1,
		             MPI_DOUBLE, left_rank, GOOD_TAG + 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		right = right_recv;
		left = left_recv;

		// .. we propagate the temperature
		for(long long i = 0; i < sec_size; i++) {
			// center stays constant (the heat is still on)
			if(relative_source_pos >= 0 && relative_source_pos < sec_size &&
			   i == relative_source_pos) {
				B[i] = SubSec[i];
				continue;
			}

			// get temperature at current position
			value_t tc = SubSec[i];

			// get temperatures of adjacent cells
			value_t tl = (i != 0) ? SubSec[i - 1] : left;
			value_t tr = (i != sec_size - 1) ? SubSec[i + 1] : right;

			// compute new temperature at current position
			B[i] = tc + 0.2 * (tl + tr + (-2 * tc));
		}

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

	// done
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
