#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// 6.4 million characters = 6.4MB of data per rank per iteration -> 10 iter -> 64MB per rank total
#define NUM_CHARS ((long long)(64 * 1000 * 100))
#define NUM_ITERATIONS 10
#define FILE_NAME "/scratch/cb761017/hpc_08/hybrid.out"

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

void check_IO(size_t target, size_t actual) {
    if(target > actual) {
        fprintf(stderr, "Unable to fully write/read data\n");
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
    size_t total_data_size = data_size * num_ranks;
	char* rank_buffer = calloc(data_size, sizeof(char));
	check_ptr(rank_buffer);

	generate_data(rank_buffer, data_size, rank);

    char* comm_buffer = NULL; 
    FILE* output = NULL;

    if(rank == 0) {
        comm_buffer = malloc(total_data_size * sizeof(char));
        check_ptr(comm_buffer);
        output = fopen(FILE_NAME, "w+");
		if(output == NULL) {
			fprintf(stderr, "Error while opening file!");
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
			return EXIT_FAILURE;
		}
    }

	// time measurement
	struct timespec start, end;
	clock_gettime(CLOCK_MONOTONIC, &start);

	fflush(stdout);
	// Write-Read-Loop
    if(rank == 0) {
        for(int i = 0; i < NUM_ITERATIONS; i++) {
            //Collect data
            MPI_Gather(rank_buffer, data_size, MPI_CHAR, comm_buffer, data_size, MPI_CHAR, 0, MPI_COMM_WORLD);
            // write data
            check_IO(total_data_size, fwrite(comm_buffer, sizeof(char), total_data_size, output));
            //  flush data to assert all data is written before reading
            fflush(output);

            if(i == NUM_ITERATIONS - 1) {
                break;
            }
            // reset file pointer to begin
            rewind(output);
            // read data
            check_IO(total_data_size, fread(comm_buffer, sizeof(char), total_data_size, output));
			// Scatter data
            MPI_Scatter(comm_buffer, data_size, MPI_CHAR, rank_buffer, data_size, MPI_CHAR, 0, MPI_COMM_WORLD);
            // reset file pointer to begin
            rewind(output);
        }
    } else {
        for(int i = 0; i < NUM_ITERATIONS; i++) {
            //Send data
            MPI_Gather(rank_buffer, data_size, MPI_CHAR, comm_buffer, data_size, MPI_CHAR, 0, MPI_COMM_WORLD);
            if(i == NUM_ITERATIONS - 1) {
                break;
            }
            // Receive data
            MPI_Scatter(comm_buffer, data_size, MPI_CHAR, rank_buffer, data_size, MPI_CHAR, 0, MPI_COMM_WORLD);
        }
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
    if(rank == 0) {
		fclose(output);
	    free(comm_buffer);
	}
	free(rank_buffer);
	MPI_Finalize();
	return EXIT_SUCCESS;
}