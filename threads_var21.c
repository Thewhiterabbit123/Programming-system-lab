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

double dt = 1, dx = 1, dy = 1, dz = 1, a = 1;
double*** matrix;
int y_grid = 0, x_grid = 0, t_grid = 0, thread_quantity = 0;;
pthread_barrier_t barr;

#define _NEXT_TIME(Z2, Z1, Z0, DELTA) (Z2-2*Z1+Z0)/DELTA
#define _STABLE(DX, A) DX*DX / 2*A

double force_impact(int x, int y, int t) {
    if(x == x_grid/2 && y == y_grid/2  && t < t_grid/5) {
      return 1;
    }
    return 0;
}

int next_time_t(int i, int j, int t) {
	matrix[i][j][(t+1) % T_IN_MEMORY] = a*a*dt*((_NEXT_TIME(matrix[i+1][j][t % T_IN_MEMORY], matrix[i][j][t % T_IN_MEMORY], matrix[i-1][j][t % T_IN_MEMORY], dx)  + 
									   			 _NEXT_TIME(matrix[i][j+1][t % T_IN_MEMORY], matrix[i][j][t % T_IN_MEMORY], matrix[i][j-1][t % T_IN_MEMORY], dy)) + 
									   			force_impact(i, j, t)) + 2*matrix[i][j][t % T_IN_MEMORY] - matrix[i][j][(t-1)%T_IN_MEMORY];
	return 0;
}

void* solver(void* arg) {
	printf("start\n");
	ThreadRecord* data = (ThreadRecord*) arg;
	for (int k = 1; k < t_grid; ++k){
		for (int i = data->start; i <  data->end; i++){
			if (!(i % x_grid == 0 || (i + 1) % x_grid == 0 || i / x_grid == 0 || (i + 1) / x_grid == 0 || i / x_grid == y_grid - 1)){
				next_time_t(i / x_grid, i % x_grid, k);
			}
		
		}
		printf("t - %d\n", k);
	//pthread_barrier_wait(&barr);
	//pthread_barrier_wait(&barr);
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

	
	/*FILE * gnu = popen("gnuplot -persistent", "w");
	fprintf(gnu, "set terminal gif animate delay 50\n");
	fprintf(gnu, "set output 'animate.gif'\n");
    
	fprintf(gnu, "set ticslevel 0\n");
	
	fprintf(gnu, "set xrange[0:%d]\n", x_grid-1);
	fprintf(gnu, "set yrange[0:%d]\n", y_grid-1);
	fprintf(gnu, "set zrange[-0.2:0.7]\n");
	  
	fprintf(gnu, "set dgrid3d %d %d \n", x_grid, y_grid);
	fprintf(gnu, "set hidden3d\n");
	 
	fprintf(gnu, "do for [i=1:%d] {\n", t_grid);
	fprintf(gnu, "splot '-' with lines\n");
	fprintf(gnu, "}\n");
	
	for (int i = 0; i < x_grid; ++i) {
	    for (int j = 0; j < y_grid; ++j) {
	      fprintf(gnu, "%lf %lf %lf\n", (double)i, (double)j, matrix[i][j][0]);
	    }
	}
	fprintf(gnu, "e\n");*/
	
	ThreadRecord* threads = NULL;
	struct timeval tv1, tv2;
	struct timezone tz;

	pthread_attr_t pattr;
	pthread_attr_init(&pattr);
	pthread_attr_setscope(&pattr, PTHREAD_SCOPE_SYSTEM);

	int thread_length = y_grid * x_grid / thread_quantity;

	if (thread_length * thread_quantity != y_grid * x_grid) {
		fprintf(stderr, "Thread count must be a divider of y_grid*x_grid!\n");
		exit(2);
	}
	
	pthread_barrier_init(&barr, NULL, thread_quantity+1);

	threads = (ThreadRecord*) calloc(thread_quantity, sizeof(ThreadRecord));

	gettimeofday(&tv1, &tz);

	for (int i = 0; i < thread_quantity; i++) {
		threads[i].start = thread_length * i + 1;
		threads[i].end = thread_length * (i + 1);

		if (pthread_create(&(threads[i].tid), &pattr, solver, (void *) &(threads[i]))) {
		    printf("%s\n", strerror(errno));
		}
	}


	
	/*for (int k = 1; k < t_grid; ++k) {
	pthread_barrier_wait(&barr);
	  for (int i = 0; i < x_grid; ++i) {
	    for (int j = 0; j < y_grid; ++j) {
	      fprintf(gnu, "%lf %lf %lf\n", (double)i, (double)j, matrix[i][j][(k+1)%3]);
	    }
	  }
	  fprintf(gnu, "e\n");
	  pthread_barrier_wait(&barr);
	}
	pclose(gnu);
	*/
	
	
	
	for (int c = 0; c < thread_quantity; c++) {
	  pthread_join(threads[c].tid, NULL);
	}


   	gettimeofday(&tv2, &tz);
	printf("%ldms\n", tv2.tv_sec - tv1.tv_sec);

	//x-10000 y-10000 z-100 - 8min, 17sec   1 thread
	//x-10000 y-10000 z-100 - 4min, 07sec   2 threads
	//x-10000 y-10000 z-100 - 2min, 05sec   4 threads
	//x-10000 y-10000 z-100 - 1min, 49 sec  8 threads


	return 0;
}
