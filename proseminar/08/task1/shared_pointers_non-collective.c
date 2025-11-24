#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 6.4 million characters = 6.4MB of data per rank per iteration -> 10 iter -> 64MB per rank total
#define NUM_CHARS ((long long)(64 * 1000 * 100))
#define NUM_ITERATIONS 10

void generate_data(char* buffer, long size, int rank) {
	for(long i = 0; i < size; i++) {
		buffer[i] = 'A' + (rank % 26); // Fill with repeating letters A-Z
	}
}

double time_diff(struct timespec start, struct timespec end) {
	return (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
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

	MPI_File mpi_file;
	MPI_File_open(MPI_COMM_WORLD, "/scratch/cb761017/hpc_08/shared_non_collective.out",
	              MPI_MODE_CREATE | MPI_MODE_RDWR | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL,
	              &mpi_file);

	// time measurement
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	// Write-Read-Loop
	for(int i = 0; i < NUM_ITERATIONS; i++) {
		// write data
		MPI_File_write_shared(mpi_file, write_buffer, data_size, MPI_CHAR, MPI_STATUS_IGNORE);
		if(i == NUM_ITERATIONS - 1) {
			break;
		}
		//  flush data to assert all data is written before reading
		MPI_File_sync(mpi_file);
		// reset file pointer to begin
		MPI_File_seek_shared(mpi_file, 0, MPI_SEEK_SET);
		// read data
		MPI_File_read_shared(mpi_file, read_buffer, data_size, MPI_CHAR, MPI_STATUS_IGNORE);
		// reset file pointer to begin
		MPI_File_seek_shared(mpi_file, 0, MPI_SEEK_SET);
	}

	// time measurement
	clock_gettime(CLOCK_MONOTONIC, &end);
	double elapsed_time = time_diff(start, end);

	double sum_time = 0;
	MPI_Reduce(&elapsed_time, &sum_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		// csv output format:
		// num_ranks, average_time_per_rank,total_bandwidth(MB/s)

		// bandwidth asssumption: each rank writes and reads NUM_CHARS bytes NUM_ITERATIONS times
		double avg_time = sum_time / (double)num_ranks;
		long double total_bandwidth = (NUM_CHARS * num_ranks * 2 * NUM_ITERATIONS) / avg_time;
		printf("%d,%lf,%Lf\n", num_ranks, avg_time, total_bandwidth / 1000000.0L); // output in MB/s
	}

	// clean up
	free(write_buffer);
	free(read_buffer);
	MPI_File_close(&mpi_file);
	MPI_Finalize();
	return EXIT_SUCCESS;
}