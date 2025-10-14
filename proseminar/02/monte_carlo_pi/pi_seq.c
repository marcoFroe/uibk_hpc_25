#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
    double inside = 0;
    srand(time(NULL));
    for(int i = 0; i < n; i++) {
        double x = (double) rand() / RAND_MAX;
        double y = (double) rand() / RAND_MAX;
        if(x * x + y * y <= 1) {
            inside++;
        }
    }
    inside = (inside / n) * 4;
    printf("Computed value for pi with %d samples: %f\n", n, inside);
    return EXIT_SUCCESS;
}
