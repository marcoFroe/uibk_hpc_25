#include <mpi.h>
#include <stdio.h>

int main(int argc, char** argv) {
    int nprocs; /* the number of processes in the task */
    int myrank; /* my rank */
    const int count = 10;
    int tag = 42;  /* tag used for all communication */
    int good_data = 1;
    int data[count];   /* data buffers */
    MPI_Status status; /* status of MPI_Recv() operation */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    /* Initialize data buffer */
    for(int i = 0; i < count; i++) {
        if(myrank == 0) {
            data[i] = 399; /* root initializes data */
        } else {
            data[i] = 0; /* others initialize to 0 (or anything, will be overwritten) */
        }
    }

    /* Root sends to all other processes */
    if(myrank == 0) {
        for(int i = 1; i < nprocs; i++) { /* start from 1, not 0 */
            MPI_Send(&data, count, MPI_INT, i, tag, MPI_COMM_WORLD);
        }
    } else {
        /* Others receive from root */
        MPI_Recv(&data, count, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Check the data everywhere */
    if(myrank != 0) {
        for(int i = 0; i < count; i++) {
            if(data[i] != 399) {
                good_data = 0;
            }
        }
        if(good_data == 0) {
            fprintf(stdout, "Whoa! The data is incorrect\n");
        } else {
            fprintf(stdout, "Rank %d received correct data\n", myrank);
        }
    }

    MPI_Finalize();
    return 0;
}
