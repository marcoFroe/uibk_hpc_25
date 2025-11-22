#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 64 million characters = 64MB of data per rank
#define NUM_CHARS 64 * 1000 * 1000
#define NUM_ITERATIONS 10

void generate_data(char* buffer, long size, int rank) {
	for(long i = 0; i < size; i++) {
		buffer[i] = 'A' + (rank % 26); // Fill with repeating letters A-Z
	}
}

double time_diff(struct timespec start, struct timespec end) {
	return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
}

FILE* get_file(int rank) {
	char filename[39];
	snprintf(filename, sizeof(filename), "/scratch/cb761017/hpc_08/rank_%02d.out", rank);
	FILE* file = fopen(filename, "w+");
	return file;
}

void check_ptr(void* ptr) {
	if(ptr == NULL) {
		fprintf(stderr, "Memory allocation failed!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
	}
}

int main(int argc, char** argv) {
	MPI_Init(&argc, &argv);
	int num_ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	// buffer generation
	size_t data_size = NUM_CHARS;
	char* write_buffer = calloc(data_size, sizeof(char));
	char* read_buffer = calloc(data_size, sizeof(char));
	check_ptr(write_buffer);
	check_ptr(read_buffer);

	generate_data(write_buffer, data_size, rank);

	FILE* rank_file = get_file(rank);
	check_ptr(rank_file);

	// time measurement
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	fflush(stdout);
	// Write-Read-Loop
	for(int i = 0; i < NUM_ITERATIONS; i++) {
		// write data
		fwrite(write_buffer, sizeof(char), data_size, rank_file);
		//  flush data to assert all data is written before reading
		fflush(rank_file);
		// read data
		fread(read_buffer, sizeof(char), data_size, rank_file);
	}

	// time measurement
	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_time = time_diff(start, end);

	double sum_time = 0;
	MPI_Reduce(&elapsed_time, &sum_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		// csv output format:
		// num_ranks, average_time_per_rank,total_bandwidth

		// bandwidth asssumption: each rank writes and reads NUM_CHARS bytes NUM_ITERATIONS times
		double avg_time = sum_time / (double)num_ranks;
		double total_bandwidth = NUM_CHARS * num_ranks * 2 * NUM_ITERATIONS / avg_time;
		printf("%d,%lf,%lf\n", num_ranks, avg_time, total_bandwidth);
	}

	// clean up
	free(write_buffer);
	free(read_buffer);
	fclose(rank_file);
	MPI_Finalize();
	return EXIT_SUCCESS;
}