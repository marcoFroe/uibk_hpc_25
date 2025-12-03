#include <mpi.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

// Include that allows to print result as an image
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define DEFAULT_SIZE_X 1280
#define DEFAULT_SIZE_Y 720

// RGB image will hold 3 color channels
#define NUM_CHANNELS 3
// max iterations cutoff
#define MAX_ITER 10000

#define IND(Y, X, SIZE_Y, SIZE_X, CHANNEL) \
	((Y) * (SIZE_X) * (NUM_CHANNELS) + (X) * (NUM_CHANNELS) + (CHANNEL))

typedef struct {
	int* recvcounts;
	int* displs;
} DistInfo;

typedef struct {
	int x;
	int y;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} cell_t;

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B);
void calcMandelbrot(cell_t* cells, int length, int global_x, int global_y);
void setupDistCalls(int total_elements, int num_ranks, DistInfo* info);
void mem_checker(void* ptr);
cell_t* prepare_data(int sizeX, int sizeY);
void shuffle_array(cell_t* cells, int length);
void create_mpi_type(MPI_Datatype* type);
void reconstruct_image(uint8_t* image, cell_t* cells, int sizeY, int sizeX);

int main(int argc, char** argv) {
	// MPI Setup
	int rank;
	int num_ranks;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

	// parameter passing and storage allocation
	int global_sizeX = DEFAULT_SIZE_X;
	int global_sizeY = DEFAULT_SIZE_Y;

	if(argc == 3) {
		global_sizeX = atoi(argv[1]);
		global_sizeY = atoi(argv[2]);
	} else if(rank == 0) {
		printf("No arguments given, using default size.\n");
	}

	int local_elems = (global_sizeX * global_sizeY) / num_ranks;
	if(rank == 0) {
		local_elems += (global_sizeX * global_sizeY) % num_ranks;
	}

	cell_t* local_cells = malloc(local_elems * sizeof(cell_t));
	mem_checker(local_cells);

	cell_t* global_cells = NULL;
	uint8_t* global_image = NULL;
	if(rank == 0) {
		global_image = malloc(sizeof(uint8_t) * global_sizeX * global_sizeY * NUM_CHANNELS);
		mem_checker(global_image);
		global_cells = prepare_data(global_sizeX, global_sizeY);
		shuffle_array(global_cells, global_sizeX * global_sizeY);
	}

	// Data distribution
	MPI_Datatype MPI_Cells_t;
	create_mpi_type(&MPI_Cells_t);

	DistInfo info;
	setupDistCalls(global_sizeX * global_sizeY, num_ranks, &info);

	MPI_Scatterv(global_cells, info.recvcounts, info.displs, MPI_Cells_t, local_cells, local_elems,
	             MPI_Cells_t, 0, MPI_COMM_WORLD);

	// Mandelbrot Calculations
	struct timeval start, end;
	gettimeofday(&start, NULL);

	calcMandelbrot(local_cells, local_elems, global_sizeX, global_sizeY);

	gettimeofday(&end, NULL);
	double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	// printf("Mandelbrot set calculation for rank %d took %lf seconds.\n", rank, timeElapsed);
	printf("%d,%lf\n", num_ranks, timeElapsed);

	// Image Construction
	double sum_time = 0;
	MPI_Reduce(&timeElapsed, &sum_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Gatherv(local_cells, local_elems, MPI_Cells_t, global_cells, info.recvcounts, info.displs,
	            MPI_Cells_t, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		reconstruct_image(global_image, global_cells, global_sizeY, global_sizeX);
		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi.png", global_sizeX, global_sizeY, NUM_CHANNELS, global_image,
		               stride_bytes);
		free(global_image);
		free(global_cells);
	}

	// Clean up
	free(local_cells);
	free(info.recvcounts);
	free(info.displs);
	MPI_Type_free(&MPI_Cells_t);
	MPI_Finalize();
	return EXIT_SUCCESS;
}

void calcMandelbrot(cell_t* cells, int length, int global_X, int global_Y) {
	const float left = -2.5, right = 1;
	const float bottom = -1, top = 1;

	for(int i = 0; i < length; i++) {
		// scale y pixel into mandelbrot coordinate system
		const float cy = (cells[i].y / (float)global_Y) * (top - bottom) + bottom;

		// scale x pixel into mandelbrot coordinate system
		const float cx = (cells[i].x / (float)global_X) * (right - left) + left;
		float x = 0;
		float y = 0;
		int numIterations = 0;

		// Check if the distance from the origin becomes
		// greater than 2 within the max number of iterations.
		while((x * x + y * y <= 2 * 2) && (numIterations < MAX_ITER)) {
			float x_tmp = x * x - y * y + cx;
			y = 2 * x * y + cy;
			x = x_tmp;
			numIterations += 1;
		}

		// Normalize iteration and write it to pixel position
		double value = fabs((numIterations / (float)MAX_ITER)) * 200;

		double red = 0;
		double green = 0;
		double blue = 0;

		HSVToRGB(value, 1.0, 1.0, &red, &green, &blue);

		cells[i].red = (uint8_t)(red * UINT8_MAX);
		cells[i].green = (uint8_t)(green * UINT8_MAX);
		cells[i].blue = (uint8_t)(blue * UINT8_MAX);
	}
}

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B) {
	if(H >= 1.00) {
		V = 0.0;
		H = 0.0;
	}

	double step = 1.0 / 6.0;
	double vh = H / step;

	int i = (int)floor(vh);

	double f = vh - i;
	double p = V * (1.0 - S);
	double q = V * (1.0 - (S * f));
	double t = V * (1.0 - (S * (1.0 - f)));

	switch(i) {
		case 0: {
			*R = V;
			*G = t;
			*B = p;
			break;
		}
		case 1: {
			*R = q;
			*G = V;
			*B = p;
			break;
		}
		case 2: {
			*R = p;
			*G = V;
			*B = t;
			break;
		}
		case 3: {
			*R = p;
			*G = q;
			*B = V;
			break;
		}
		case 4: {
			*R = t;
			*G = p;
			*B = V;
			break;
		}
		case 5: {
			*R = V;
			*G = p;
			*B = q;
			break;
		}
	}
}

void setupDistCalls(int total_elements, int num_ranks, DistInfo* info) {
	info->recvcounts = malloc((size_t)num_ranks * sizeof(int));
	info->displs = malloc((size_t)num_ranks * sizeof(int));
	mem_checker(info->recvcounts);
	mem_checker(info->displs);

	int base = total_elements / num_ranks;
	int rem = total_elements % num_ranks;

	int offset = 0;
	for(int r = 0; r < num_ranks; r++) {
		int count = base + (r < rem ? 1 : 0);
		info->recvcounts[r] = count;
		info->displs[r] = offset;
		offset += count;
	}
}

void mem_checker(void* ptr) {
	if(ptr == NULL) {
		fprintf(stderr, "Error while allocating memory!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		exit(EXIT_FAILURE);
	}
}

cell_t* prepare_data(int sizeX, int sizeY) {
	cell_t* cells = calloc(sizeX * sizeY, sizeof(cell_t));
	mem_checker(cells);

	for(int itr_y = 0; itr_y < sizeY; itr_y++) {
		for(int itr_x = 0; itr_x < sizeX; itr_x++) {
			int idx = itr_y * sizeX + itr_x;
			cells[idx].x = itr_x;
			cells[idx].y = itr_y;
		}
	}

	return cells;
}

void shuffle_array(cell_t* cells, int length) {
	if(length <= 1) return;

	srand(time(NULL));

	for(int i = length - 1; i > 0; i--) {
		int j = rand() % (i + 1);

		cell_t tmp = cells[i];
		cells[i] = cells[j];
		cells[j] = tmp;
	}
}

void create_mpi_type(MPI_Datatype* type) {
	// vector_t
	int block_lengths_vec[5] = { 1, 1, 1, 1, 1 };
	MPI_Aint displacements_vec[5] = {
		offsetof(cell_t, x),     offsetof(cell_t, y),    offsetof(cell_t, red),
		offsetof(cell_t, green), offsetof(cell_t, blue),
	};
	MPI_Datatype datatypes_vec[5] = { MPI_INT, MPI_INT, MPI_UINT8_T, MPI_UINT8_T, MPI_UINT8_T };

	MPI_Type_create_struct(5, block_lengths_vec, displacements_vec, datatypes_vec, type);
	MPI_Type_commit(type);
}

void reconstruct_image(uint8_t* image, cell_t* cells, int sizeY, int sizeX) {
	int len = sizeX * sizeY;
	for(int i = 0; i < len; i++) {

		int channel = 0;
		image[IND(cells[i].y, cells[i].x, sizeY, sizeX, channel++)] = cells[i].red;
		image[IND(cells[i].y, cells[i].x, sizeY, sizeX, channel++)] = cells[i].green;
		image[IND(cells[i].y, cells[i].x, sizeY, sizeX, channel++)] = cells[i].blue;
	}
}