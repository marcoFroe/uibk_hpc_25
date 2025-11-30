#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

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
} GatherInfo;

void HSVToRGB(double H, double S, double V, double* R, double* G, double* B);
void calcMandelbrot(uint8_t* image, int sizeX, int sizeY, int rank, int global_sizeY);
GatherInfo setupDistCalls(int global_X, int global_Y, int num_ranks);
void mem_checker(void* ptr);

int main(int argc, char** argv) {
	int rank;
	int num_ranks;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

	int global_sizeX = DEFAULT_SIZE_X;
	int global_sizeY = DEFAULT_SIZE_Y;

	if(argc == 3) {
		global_sizeX = atoi(argv[1]);
		global_sizeY = atoi(argv[2]);
	} else if(rank == 0) {
		printf("No arguments given, using default size.\n");
	}

	int local_sizeX = global_sizeX;
	int local_sizeY = global_sizeY / num_ranks;

	if(rank == 0) {
		local_sizeY += global_sizeY % num_ranks;
	}

	int local_elems = NUM_CHANNELS * local_sizeX * local_sizeY;

	uint8_t* local_image = malloc(local_elems * sizeof(uint8_t));
	mem_checker(local_image);

	uint8_t* global_image = NULL;
	if(rank == 0) {
		global_image = malloc(NUM_CHANNELS * global_sizeX * global_sizeY * sizeof(uint8_t));
		mem_checker(global_image);
	}

	struct timeval start, end;
	gettimeofday(&start, NULL);

	calcMandelbrot(local_image, local_sizeX, local_sizeY, rank, global_sizeY);

	gettimeofday(&end, NULL);
	double timeElapsed = (end.tv_sec + end.tv_usec * 1e-6) - (start.tv_sec + start.tv_usec * 1e-6);
	// printf("Mandelbrot set calculation for rank %d took %lf seconds.\n", rank, timeElapsed);
	printf("%d,%lf\n", num_ranks, timeElapsed);

	GatherInfo info;
	if(rank == 0) {
		info = setupDistCalls(global_sizeX, global_sizeY, num_ranks);
	}

	double sum_time = 0;
	MPI_Reduce(&timeElapsed, &sum_time, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
	MPI_Gatherv(local_image, local_elems, MPI_UINT8_T, global_image, info.recvcounts, info.displs,
	            MPI_UINT8_T, 0, MPI_COMM_WORLD);

	if(rank == 0) {
		// printf("Mandelbrot set calculation for %dx%d took on average: %lf seconds.\n",
		// global_sizeX,global_sizeY, sum_time / num_ranks);

		const int stride_bytes = 0;
		stbi_write_png("mandelbrot_mpi.png", global_sizeX, global_sizeY, NUM_CHANNELS, global_image,
		               stride_bytes);
		free(global_image);
	}

	free(local_image);
	MPI_Finalize();
	return EXIT_SUCCESS;
}

void calcMandelbrot(uint8_t* image, int sizeX, int sizeY, int rank, int global_sizeY) {
	const float left = -2.5, right = 1;
	const float bottom = -1, top = 1;

	int global_y = sizeY * rank;

	for(int pixelY = 0; pixelY < sizeY; pixelY++, global_y++) {
		// scale y pixel into mandelbrot coordinate system
		const float cy = (global_y / (float)global_sizeY) * (top - bottom) + bottom;
		for(int pixelX = 0; pixelX < sizeX; pixelX++) {
			// scale x pixel into mandelbrot coordinate system
			const float cx = (pixelX / (float)sizeX) * (right - left) + left;
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

			int channel = 0;
			image[IND(pixelY, pixelX, sizeY, sizeX, channel++)] = (uint8_t)(red * UINT8_MAX);
			image[IND(pixelY, pixelX, sizeY, sizeX, channel++)] = (uint8_t)(green * UINT8_MAX);
			image[IND(pixelY, pixelX, sizeY, sizeX, channel++)] = (uint8_t)(blue * UINT8_MAX);
		}
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

GatherInfo setupDistCalls(int global_X, int global_Y, int num_ranks) {
	GatherInfo info;
	info.recvcounts = malloc(num_ranks * sizeof(int));
	info.displs = malloc(num_ranks * sizeof(int));

	int offset = 0;

	for(int r = 0; r < num_ranks; r++) {
		int sizeY = global_Y / num_ranks;
		if(r == 0) {
			sizeY += global_Y % num_ranks;
		}

		info.recvcounts[r] = NUM_CHANNELS * global_X * sizeY;
		info.displs[r] = offset;
		offset += info.recvcounts[r];
	}
	return info;
}

void mem_checker(void* ptr) {
	if(ptr == NULL) {
		fprintf(stderr, "Error while allocating memory!\n");
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		exit(EXIT_FAILURE);
	}
}
