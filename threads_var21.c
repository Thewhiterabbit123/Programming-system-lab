#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h> 
#include <sys/time.h>
#include <errno.h>
#include <string.h>


#define T_IN_MEMORY 3

typedef struct {
	pthread_t tid;
	int start;
	int end;
} ThreadRecord;

double dt = 1, dx = 1, dy = 1, dz = 1, a= 1;
double*** matrix;
int y_grid = 0, x_grid = 0, t_grid = 0, thread_quantity = 0;;
pthread_barrier_t barr;

#define _FORCE_IMPACT(X, Y, T) X+Y+T
#define _NEXT_TIME(Z2, Z1, Z0, DELTA) (Z2-2*Z1+Z0)/DELTA
#define _STABLE(DX, A) DX*DX / 2*A

int next_time_t(int i, int j, int t) {
	matrix[i][j][(t+1) % T_IN_MEMORY] = a*a*dt*((_NEXT_TIME(matrix[i+1][j][t % T_IN_MEMORY], matrix[i][j][t % T_IN_MEMORY], matrix[i-1][j][t % T_IN_MEMORY], dx)  + 
									   			 _NEXT_TIME(matrix[i][j+1][t % T_IN_MEMORY], matrix[i][j][t % T_IN_MEMORY], matrix[i][j-1][t % T_IN_MEMORY], dy)) + 
									   			 _FORCE_IMPACT(i, j, t)) + 2*matrix[i][j][t % T_IN_MEMORY] - matrix[i][j][(t-1)%T_IN_MEMORY];
	return 0;
}


int gnu_plot_result() {
	FILE *gnu = popen("gnuplot -persist", "w");
	fprintf (gnu, "set dgrid3d\n");
	fprintf (gnu, "set hidden3d\n");
	fprintf (gnu, "splot '-' using 1:2:3 with lines\n");

	double x_range = 0;
	double y_range = 0;

	for (int j = 0; j < y_grid; j++, y_range += dy) {
		for (int i = 0; i < x_grid; i++, x_range += dx) {
			fprintf(gnu, "%lf %lf %lf\n", x_range, y_range, matrix[i][j][(t_grid+1)%T_IN_MEMORY]);
		}
		x_range = 0;
		fprintf (gnu, "\n");
	}
	fprintf (gnu, "e\n");

	pclose(gnu);
	return 0;
}

void* solver(void* arg) {
	ThreadRecord* data = (ThreadRecord*) arg;
	for (int k = 1; k < t_grid; ++k){
		for (int i = data->start; i <  data->end; i++){
			if (!(i % x_grid == 0 || (i + 1) % x_grid == 0 || i / x_grid == 0 || (i + 1) / x_grid == 0 || i / x_grid == y_grid - 1)){
				next_time_t(i / x_grid, i % x_grid, k);
			}	
		}
		pthread_barrier_wait(&barr);
	}
	
	return NULL;
}

void matrix_memory() {
	printf("Enter y_grid, x_grid, t_grid\n");
	scanf("%d%d%d", &y_grid, &x_grid, &t_grid);

	matrix = (double***)calloc(y_grid, sizeof(double));

	if(!matrix) {
		printf("%s\n", strerror(errno));
	}

	for (int i = 0; i < y_grid; ++i) {
		*(matrix + i) = (double**)calloc(x_grid, sizeof(double));
		for (int k = 0; k < x_grid; ++k) {
			*(*(matrix+i) + k) = (double*)calloc(T_IN_MEMORY, sizeof(double));
		}
	}

	printf("Enter a, dx, dy, dt and quantity of threads\n");
	scanf("%lf%lf%lf%lf%d", &a, &dx, &dy, &dz, &thread_quantity);

	dt = _STABLE(dx, a);

	return (void)0;
}

void matrix_free() {
	
	for (int i = 0; i < y_grid; ++i) {
		free(*(matrix + i));
	}
	free(matrix);
}

int main(int argc, char const *argv[]) {
	
	matrix_memory();


	ThreadRecord* threads = NULL;
	struct timeval tv1, tv2;
	struct timezone tz;

	pthread_attr_t pattr;
	pthread_attr_init(&pattr);
	pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);

	int thread_length = y_grid * x_grid / thread_quantity;

	if (thread_length * thread_quantity != y_grid * x_grid) {
		fprintf(stderr, "Thread count must be a multiple of y_grid*x_grid! Exit...\n");
		exit(2);
	}
	
	pthread_barrier_init(&barr, NULL, thread_quantity);

	threads = (ThreadRecord*) calloc(thread_quantity, sizeof(ThreadRecord));

	gettimeofday(&tv1, &tz);

	for (int i = 0; i < thread_quantity; i++) {
		threads[i].start = thread_length * i + 1;
		threads[i].end = thread_length * (i + 1);

		if (pthread_create(&(threads[i].tid), &pattr, solver, (void *) &(threads[i]))) {
		    printf("%s\n", strerror(errno));
		}
	}

	for (int c = 0; c < thread_quantity; c++) {
        pthread_join(threads[c].tid, NULL);
    }


   	gettimeofday(&tv2, &tz);
	printf("%ldms\n", tv2.tv_usec - tv1.tv_usec);


	gnu_plot_result();

	return 0;
}
