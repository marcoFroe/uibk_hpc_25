#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define G (6.67430e-11) // actual Gravitational constant
#define MAX_POS 10      // Corner
// #define MAX_POS 100  //uniform
#define MAX_VELO 1
#define MAX_MASS 25000
#define FILE_NAME "data.dat"
#define DT (0.1)
// #define PRINT_SIM

typedef struct {
	double x;
	double y;
	double z;
} vector_t;

typedef struct {
	double mass;
	vector_t pos;
	vector_t velo;
} particle_t;

MPI_Datatype MPI_VECTOR_T;

vector_t calcForceVector(particle_t p1, particle_t p2);
double calcDistance(vector_t pos1, vector_t pos2);
double calcForceComp(double m1, double m2, double pos1, double pos2, double r);
vector_t calcForceVector(particle_t p1, particle_t p2);
double calcVelocityComp(double m, double velocity, double force);
vector_t calcVelocityVector(vector_t oldVelocity, double mass, vector_t forceVector);
vector_t calcPositionVector(vector_t oldPos, vector_t velocity);
void updateParticle(particle_t* particle, vector_t force);
void negateVector(vector_t* force);
void sumVectors(vector_t* v1, vector_t* v2);
void randomVector(vector_t* vector, int max_value);
int parser(char* toParse);
void writeToFile(vector_t* pos, FILE* datafile);
void printParticles(particle_t* particles, int num_particles, FILE* output);
void create_mpi_type(void);

int main(int argc, char** argv) {
	if(argc != 3) {
		fprintf(stderr, "Invalid amount of arguments. Usage %s <num_particles> <timesteps>",
		        argv[0]);
		return EXIT_FAILURE;
	}

	MPI_Init(&argc, &argv);
	int num_ranks;
	MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	create_mpi_type();

#ifdef PRINT_SIM
	FILE* output = NULL;
	if(rank == 0) {
		output = fopen(FILE_NAME, "w");
		if(output == NULL) {
			fprintf(stderr, "Error while opening file!");
			MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
			return EXIT_FAILURE;
		}
	}
#endif

	int total_num_particles = parser(argv[1]);

	// padding so that each rank has equal particles:
	if(total_num_particles % num_ranks != 0) {
		total_num_particles += num_ranks - (total_num_particles % num_ranks);
	}
	int rank_particles = total_num_particles / num_ranks;

	int time_steps = parser(argv[2]);

	particle_t* particles = malloc(sizeof(particle_t) * total_num_particles);
	if(particles == NULL) {
		fprintf(stderr, "Error while allocating memory!");
		return EXIT_FAILURE;
	}

	srand(time(NULL) + 100 * num_ranks);
	for(int i = rank_particles * rank; i < rank_particles * (rank + 1); i++) {
		particles[i].mass = (rand() % MAX_MASS) + 1;
		randomVector(&particles[i].pos, MAX_POS);
		randomVector(&particles[i].velo, MAX_VELO);
	}

	// gather all initial positions and mass and apply to all particles
	vector_t* temp_pos = malloc(sizeof(vector_t) * total_num_particles);
	if(temp_pos == NULL) {
		fprintf(stderr, "Error while allocating memory!");
		free(particles);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	double* temp_mass = malloc(sizeof(double) * total_num_particles);
	if(temp_mass == NULL) {
		fprintf(stderr, "Error while allocating memory!");
		free(particles);
		free(temp_pos);
		MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
		return EXIT_FAILURE;
	}
	MPI_Allgather(&particles[rank_particles * rank].mass, rank_particles, MPI_DOUBLE, temp_mass,
	              rank_particles, MPI_DOUBLE, MPI_COMM_WORLD);

	MPI_Allgather(&particles[rank_particles * rank].pos, rank_particles, MPI_VECTOR_T, temp_pos,
	              rank_particles, MPI_VECTOR_T, MPI_COMM_WORLD);
	for(int i = 0; i < total_num_particles; i++) {
		particles[i].pos = temp_pos[i];
		particles[i].mass = temp_mass[i];
	}
	free(temp_pos);
	free(temp_mass);

#ifdef PRINT_SIM
	if(rank == 0) {
		printParticles(particles, total_num_particles, output);
	}
#endif

	vector_t* total_forces = calloc(total_num_particles, sizeof(vector_t));
	if(total_forces == NULL) {
		fprintf(stderr, "Error while allocating memory!");
		free(particles);
		return EXIT_FAILURE;
	}

	for(int t = 0; t < time_steps; t++) {
		for(int outer = 0; outer < total_num_particles; outer++) {
			for(int inner = rank_particles * rank; inner < rank_particles * (rank + 1); inner++) {
				vector_t forceVector = calcForceVector(particles[inner], particles[outer]);
				sumVectors(total_forces + inner, &forceVector);
			}
		}
		MPI_Allgather(total_forces + (rank_particles * rank), rank_particles, MPI_VECTOR_T,
		              total_forces, rank_particles, MPI_VECTOR_T, MPI_COMM_WORLD);

		// update all particles
		for(int i = 0; i < total_num_particles; i++) {
			updateParticle(particles + i, total_forces[i]);
			total_forces[i].x = 0;
			total_forces[i].y = 0;
			total_forces[i].z = 0;
		}

#ifdef PRINT_SIM
		if((time_steps % 10) == 0 && rank == 0) {
			printParticles(particles, total_num_particles, output);
		}
#endif
	}

#ifdef PRINT_SIM
	if(rank == 0) {
		fclose(output);
	}
#endif
	free(total_forces);
	free(particles);
	MPI_Type_free(&MPI_VECTOR_T);
	MPI_Finalize();
	return EXIT_SUCCESS;
}

double calcDistance(vector_t pos1, vector_t pos2) {
	return sqrt((pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y) +
	            (pos1.z - pos2.z) * (pos1.z - pos2.z));
}

double calcForceComp(double m1, double m2, double pos1, double pos2, double r) {
	double eps = 1e-3; // softening to mitigate division by 0
	double denom = pow(r * r + eps * eps, 1.5);
	return (G * m1 * m2 * (pos2 - pos1) / denom);
}

vector_t calcForceVector(particle_t p1, particle_t p2) {
	double distance = calcDistance(p1.pos, p2.pos);
	vector_t force = { calcForceComp(p1.mass, p2.mass, p1.pos.x, p2.pos.x, distance),
		               calcForceComp(p1.mass, p2.mass, p1.pos.y, p2.pos.y, distance),
		               calcForceComp(p1.mass, p2.mass, p1.pos.z, p2.pos.z, distance) };
	return force;
}

double calcVelocityComp(double m, double velocity, double force) {
	return velocity + (force / m) * DT;
}

vector_t calcVelocityVector(vector_t oldVelocity, double mass, vector_t forceVector) {
	vector_t newVelocity = { calcVelocityComp(mass, oldVelocity.x, forceVector.x),
		                     calcVelocityComp(mass, oldVelocity.y, forceVector.y),
		                     calcVelocityComp(mass, oldVelocity.z, forceVector.z)

	};
	return newVelocity;
}

vector_t calcPositionVector(vector_t oldPos, vector_t velocity) {
	vector_t newPos = { oldPos.x + velocity.x * DT, oldPos.y + velocity.y * DT,
		                oldPos.z + velocity.z * DT };
	return newPos;
}

void updateParticle(particle_t* particle, vector_t force) {
	particle->velo = calcVelocityVector(particle->velo, particle->mass, force);
	particle->pos = calcPositionVector(particle->pos, particle->velo);
}

void negateVector(vector_t* force) {
	force->x = -force->x;
	force->y = -force->y;
	force->z = -force->z;
}

void randomVector(vector_t* v, int max_value) {
	v->x = ((double)rand() / RAND_MAX) * 2 * max_value - max_value;
	v->y = ((double)rand() / RAND_MAX) * 2 * max_value - max_value;
	v->z = ((double)rand() / RAND_MAX) * 2 * max_value - max_value;
}

int parser(char* toParse) {
	char* endptr;
	int res = strtol(toParse, &endptr, 10);
	if(*endptr != '\0' || endptr == toParse) {
		fprintf(stderr, "Unable to parse input: %s\n", toParse);
		exit(EXIT_FAILURE);
	}
	return res;
}

void sumVectors(vector_t* v1, vector_t* v2) {
	v1->x += v2->x;
	v1->y += v2->y;
	v1->z += v2->z;
}

void writeToFile(vector_t* pos, FILE* datafile) {
	fprintf(datafile, "%lf %lf %lf\n", pos->x, pos->y, pos->z);
}

void printParticles(particle_t* particles, int num_particles, FILE* output) {
	for(int i = 0; i < num_particles; i++) {
		writeToFile(&particles[i].pos, output);
	}
	fprintf(output, "\n\n");
	fflush(output);
}

void create_mpi_type(void) {
	// vector_t
	int block_lengths_vec[3] = { 1, 1, 1 };
	MPI_Aint displacements_vec[3] = {
		offsetof(vector_t, x),
		offsetof(vector_t, y),
		offsetof(vector_t, z),
	};
	MPI_Datatype datatypes_vec[3] = { MPI_DOUBLE, MPI_DOUBLE, MPI_DOUBLE };

	MPI_Type_create_struct(3, block_lengths_vec, displacements_vec, datatypes_vec, &MPI_VECTOR_T);
	MPI_Type_commit(&MPI_VECTOR_T);
}
