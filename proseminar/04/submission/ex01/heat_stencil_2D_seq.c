#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// based on the heat stencil 2D implementation from the parallel programming ps
#define IND(y, x) ((y) * (N) + (x))
// #define MIN(x, y) (x > y ? y : x)
#define RESOLUTION_WIDTH 50
#define RESOLUTION_HEIGHT 50

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

int main(int argc, char** argv) {
    if(argc != 3) {
        fprintf(stderr, "Usage: ./seq <n> <t>\n");
        return EXIT_FAILURE;
    }
    int N = atoi(argv[1]);
    if(N < 2) {
        fprintf(stderr, "N needs to be greater than 1\n");
        return EXIT_FAILURE;
    }

    int T = atoi(argv[2]);

    double* current = malloc(sizeof(double) * N * N);
    if(current == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        return EXIT_FAILURE;
    }

    double* next = malloc(sizeof(double) * N * N);
    if(next == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        free(current);
        return EXIT_FAILURE;
    }

    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            current[IND(i, j)] = 273; 
            next[IND(i, j)] = 273; 
        }
    }

    srand(time(NULL));
    int source_x = rand() % N;
    int source_y = rand() % N;

    current[IND(source_x, source_y)] = 333;
    next[IND(source_x, source_y)] = 333;

    /*
    Locally, this has 0 impact on the program runtime.

    int lazy_steps = MIN(MIN(N - source_x - 1, source_x), MIN(N - source_y - 1, source_y)) - 1;
    for(int t = 1; t < lazy_steps; t++) {
        for(int i = source_y - t; i <= source_y + t && i < N; i++) {
            for(int j = source_x - (i - (source_y - t)); j <= source_x + (i - (source_y - t)) && j < N; j++) {
                if(i == source_x && j == source_y) {
                    continue;
                }
                double up = current[IND(i - 1, j)];
                double down = current[IND(i + 1, j)];
                double left = current[IND(i, j - 1)];
                double right = current[IND(i, j + 1)];
                double here = current[IND(i, j)];


                next[IND(i, j)] = here + 0.2 * (up + down + left + right + (-4 * here));
            }
        }

        double* temp = current;
        current = next;
        next = temp;
    }


    for(int t = lazy_steps - 1; t < T; t++) {
    */

    for(int t = 0; t < T; t++) {
        // calculate first row on its own
        double here = current[IND(0, 0)];
        next[IND(0, 0)] = here + 0.2 * (546 + current[IND(1, 0)] + current[IND(0, 1)] - 4 * here);
        for(int j = 1; j < N - 1; j++) {        
            double up = current[IND(0, j)];
            double down = current[IND(1, j)];
            double left = current[IND(0, j - 1)];
            double right = current[IND(0, j + 1)];
            here = current[IND(0, j)];

            next[IND(0, j)] = here + 0.2 * (up + down + left + right + (-4 * here));
        }
        here = current[IND(0, N - 1)];
        next[IND(0, N - 1)] = here + 0.2 * (546 + current[IND(1, N - 1)] + current[IND(0, N - 2)] - 4 * here);

        // calculate middle rows
        for(int i = 1; i < N - 1; i++) {
            here = current[IND(i, 0)];
            next[IND(i, 0)] = here + 0.2 * (273 + current[IND(i + 1, 0)] + current[IND(i, 1)] + current[IND(i - 1, 0)] - 4 * here);
            for(int j = 0; j < N; j++) {
                double up = current[IND(i - 1, j)];
                double down = current[IND(i + 1, j)];
                double left = current[IND(i, j - 1)];
                double right = current[IND(i, j + 1)];
                here = current[IND(i, j)];


                next[IND(i, j)] = here + 0.2 * (up + down + left + right + (-4 * here));
            }
            here = current[IND(i, N - 1)];
            next[IND(i, N - 1)] = here + 0.2 * (273 + current[IND(i + 1, N - 1)] + current[IND(i, N - 2)] + current[IND(i - 1, N - 1)] - 4 * here);
        }

        // calculate final row on its own
        here = current[IND(N - 1, 0)];
        next[IND(N - 1, 0)] = here + 0.2 * (546 + current[IND(N - 2, 0)] + current[IND(N - 1, 1)] - 4 * here);
        for(int j = 1; j < N - 1; j++) {
            int i = N - 1;

            double up = current[IND(i - 1, j)];
            double down = current[IND(i, j)];
            double left = (j > 0) ? current[IND(i, j - 1)] : current[IND(i, j)];
            double right = (j < N - 1) ? current[IND(i, j + 1)] : current[IND(i, j)];
            here = current[IND(i, j)];


            next[IND(i, j)] = here + 0.2 * (up + down + left + right + (-4 * here));
        }
        here = current[IND(N - 1, N - 1)];
        next[IND(N - 1, N - 1)] = here + 0.2 * (546 + current[IND(N - 2, N - 1)] + current[IND(N - 1, N - 2)] - 4 * here);

        next[IND(source_x, source_y)] = 333;        
        
        double* temp = current;
        current = next;
        next = temp;
    }

    int success = 1;
	for(int i = 0; i < N; i++) {
		for(int j = 0; j < N; j++) {
			double temp = current[IND(i, j)];
			if(273 <= temp && temp <= 273 + 60) continue;
			success = 0;
			break;
		}
	}

    printf("Final:");
	printTemperature(current, N, N);
	printf("\n");

    printf("Verification: %s\n", (success) ? "OK" : "FAILED");

    free(current);
    free(next);
    return (success) ? EXIT_SUCCESS : EXIT_FAILURE;
}


