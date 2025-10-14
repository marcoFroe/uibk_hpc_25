#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int parser(char* parsed) {
	char* endptr;
	int res = strtol(parsed, &endptr, 10);
	if(*endptr != '\0' || endptr == parsed) {
		fprintf(stderr, "Unable to parse input: %s\n", parsed);
		exit(EXIT_FAILURE);
	}
	return res;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: ./pi_seq <n>\n");
        return EXIT_FAILURE;
    }
    int n = parser(argv[1]);

    MPI_Init(&argc, &argv);
    int n_ranks;
    MPI_Comm_size(MPI_COMM_WORLD, &n_ranks);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rank_share = n / n_ranks;
    if(rank == 0) {
        rank_share += n % n_ranks;
    }
    double inside = 0;
    double total_inside;
    srand(time(NULL) + 100 * rank);

    for(int i = 0; i < rank_share; i++) {
        double x = (double) rand() / RAND_MAX;
        double y = (double) rand() / RAND_MAX;
        if(x * x + y * y <= 1) {
            inside++;
        }
    }

    MPI_Reduce(&inside, &total_inside, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if(rank == 0) {
        total_inside = (total_inside / n) * 4;
        printf("Computed value for pi with %d ranks and %d samples: %f\n", n_ranks, n, total_inside);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
