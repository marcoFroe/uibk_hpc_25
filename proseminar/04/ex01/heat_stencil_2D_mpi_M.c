#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// based on the heat stencil 2D implementation from the parallel programming ps
#define IND(y, x) ((y) * (N) + (x))
// #define DEBUG

#define RESOLUTION_WIDTH 20
#define RESOLUTION_HEIGHT 20

void printTemperature(double* m, int N, int M);
void check_malloc(void* ptr);
int temperatureCheck(double* temp_array, int sec_size, int N);

int main(int argc, char** argv) {
	// parameter parsing
	if(argc != 3) {
		fprintf(stderr, "Usage: %s <n> <t>\n", argv[0]);
		return EXIT_FAILURE;
	}
	int N = atoi(argv[1]);
	if(N < 2) {
		fprintf(stderr, "N needs to be greater than 1\n");
		return EXIT_FAILURE;
	}

	int T = atoi(argv[2]);
	if(T < 1) {
		fprintf(stderr, "T must be greater than 0.\n");
		return EXIT_FAILURE;
	}

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
		exit(EXIT_FAILURE);
	}

	// space allocation
	int sec_size = N / num_ranks;

	double* current = malloc(sizeof(double) * (sec_size)*N);
	check_malloc(current);

	double* next = malloc(sizeof(double) * (sec_size)*N);
	check_malloc(next);

	// inital temperature setting
	for(int y = 0; y < sec_size; y++) {
		for(int x = 0; x < N; x++) {
			current[IND(y, x)] = 273;
		}
	}

	int source[2];
	if(rank == 0) {
		srand(time(NULL));
		source[0] = rand() % N; // y
		source[1] = rand() % N; // x
	}

	MPI_Bcast(source, 2, MPI_INT, 0, MPI_COMM_WORLD);
	int upper_rank = (rank == 0) ? MPI_PROC_NULL : rank - 1;
	int lower_rank = (rank == num_ranks - 1) ? MPI_PROC_NULL : rank + 1;

	int relative_source_y = source[0] - (rank * sec_size);

	// setting source in the subsection needed
	if(relative_source_y >= 0 && relative_source_y < sec_size) {
		current[IND(relative_source_y, source[1])] = 333;
		next[IND(relative_source_y, source[1])] = 333;
	}

	// ghost cells setup
	double* ghost_up = malloc(sizeof(double) * N);
	check_malloc(ghost_up);
	double* ghost_down = malloc(sizeof(double) * N);
	check_malloc(ghost_down);

	for(int i = 0; i < N; i++) {
		ghost_down[i] = 273;
		ghost_up[i] = 273;
	}

	for(int t = 0; t < T; t++) {
		MPI_Sendrecv(current + (sec_size - 1) * N, N, MPI_DOUBLE, lower_rank, 42, ghost_up, N,
		             MPI_DOUBLE, upper_rank, 42, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		MPI_Sendrecv(current, N, MPI_DOUBLE, upper_rank, 43, ghost_down, N, MPI_DOUBLE, lower_rank,
		             43, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

		// temperature propagation
		for(int y = 0; y < sec_size; y++) {
			for(int x = 0; x < N; x++) {
				if(y == relative_source_y && x == source[1]) {
					next[IND(y, x)] = current[IND(y, x)];
					continue;
				}

				double up = (y == 0) ? ghost_up[x] : current[IND(y - 1, x)];
				double down = (y == sec_size - 1) ? ghost_down[x] : current[IND(y + 1, x)];
				double left = (x == 0) ? current[IND(y, x)] : current[IND(y, x - 1)];
				double right = (x == N - 1) ? current[IND(y, x)] : current[IND(y, x + 1)];
				double here = current[IND(y, x)];

				next[IND(y, x)] = here + 0.2 * (up + down + left + right + (-4 * here));
			}
		}

		double* temp = current;
		current = next;
		next = temp;
	}

	// temperature check -> distributed
	int success = temperatureCheck(current, sec_size, N);

#ifdef DEBUG
	// result print
	double* aggr = NULL;
	if(rank == 0) {
		aggr = malloc(sizeof(double) * N * N);
		check_malloc(aggr);
	}

	MPI_Gather(current, N * sec_size, MPI_DOUBLE, aggr, N * sec_size, MPI_DOUBLE, 0,
	           MPI_COMM_WORLD);
	if(rank == 0) {
		printTemperature(aggr, N, N);
	}

	// Verification output
	int test;
	MPI_Reduce(&success, &test, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	if(rank == 0) {
		printf("Verification: %s\n", success - num_ranks ? "OK" : "Failed");
	}
#endif

	MPI_Finalize();
	free(current);
	free(next);
	free(ghost_down);
	free(ghost_up);

	return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}

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

void check_malloc(void* ptr) {
	if(ptr == NULL) {
		fprintf(stderr, "Memory allocation error\n");
		MPI_Finalize();
		exit(EXIT_FAILURE);
	}
}

int temperatureCheck(double* temp_array, int sec_size, int N) {
	for(int y = 0; y < sec_size; y++) {
		for(int x = 0; x < N; x++) {
			double temp = temp_array[IND(y, x)];
			if(temp < 273 || temp > 333) {
				return 0;
			}
		}
	}
	return 1;
}
