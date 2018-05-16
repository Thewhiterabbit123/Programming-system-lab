#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h> 
#include <string.h>
#include <signal.h>
#define A 1		//фазовая скорость
#define dx 0.5	//шаг по длине
#define dy 0.5	//шаг по ширине
#define dt 0.1	//шаг по времени

struct timeval start, stop;
double timer=0;

int width = 31;
int height = 31;
int z = 30;

double force_impact(int x, int y, int t, int n, int m) {
	if (x == n/2 && y == m/2 && t*10 < (int)(z/dt)){
      	return 200;
    }
    return 0;
}

FILE *fp;
void calculate(double* zc, double* fz, double* zp, int rank, int size, int n, int m, int t) {
	int k = rank * n * m / size, kol=0;//начало в общем массиве?
	int to, from; 

	to = rank * m / size;	//от по у
	from = (m / size) * (rank + 1);	//до по у

	if(size >= 1 && rank == 0 ) {
		k = k + n;
	}		
	for(int j = to; j < from; j++) {
		for(int i = 0; i < n; i++) {
			if(j == 0 || j == m - 1) {
				fz[kol]=0;
				kol++;
			} else {
				if(i == 0 || i == n - 1 || kol % n == 0 || kol/n == n-1){
					printf("lol");
					fz[kol]=0; 
					k++;
					kol++;
				} else {
				        fz[kol]=A*A*dt*dt*((zc[k+1] - 2 * zc[k] + zc[k-1])/(dx*dx) + (zc[k+n] - 2 * (zc[k]) + zc[k-n])/(dy*dy)) + force_impact(i, j, t, n, m)*(dt*dt) + 2 * zc[k] - zp[k];
					k++;
					kol++;
				}
			}
		}
	}
}

int main(int argc, char* argv[]) {
	MPI_Init(&argc, &argv);	//Инициализация коммуникационных средств
	
	int size, rank, count=0;

	MPI_Comm_rank(MPI_COMM_WORLD, &rank);	//идентификатор процесса в группе
	MPI_Comm_size(MPI_COMM_WORLD, &size);	//Определение общего количества процессов
	printf("State0-%d\n",rank);
	
	printf("Enter width, height, time\n");
	scanf("%d%d%d", &width, &height, &z);

	
	
	int n = width / dx;	//количество шагов по длине
	int m = height / dy;	//количество шагов по ширине
	double* zp = (double*)malloc(sizeof(double) * n * m);	//прошлое
	double* zc = (double*)malloc(sizeof(double) * n * m);	//настоящее
	double* zf = (double*)malloc(sizeof(double) * n * m);	//будущее
	if (rank == 0) {	//в случае корневого процесса
		zf = (double*)malloc(sizeof(double) * n * m);
		fp = fopen("vgraph.txt","w"); 
		fprintf(fp,"%d\n",0); 
		for(double w = 0; w <= height - 1; w += dy)  {
			for(double a = 0; a <= width - 1; a += dx) {
				fprintf(fp,"%f\t%f\t%f\n",a,w,zc[count]);
				count++;
			}	
		}
		gettimeofday(&start,NULL);
	}
	double* fz=(double*)malloc(sizeof(double)*n*m/size);
        
	for(int q = 1; q < (int)(z/dt); q++) {	
		MPI_Bcast(zc, n*m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		MPI_Bcast(zp, n*m, MPI_DOUBLE, 0, MPI_COMM_WORLD);
		double* cz=(double*)malloc(sizeof(double) * n * m);
		double* pz=(double*)malloc(sizeof(double) * n * m);

		memcpy(cz, zc, sizeof(double) * n * m);
		memcpy(pz, zp, sizeof(double) * n * m);

		if(!rank) {
			fprintf(fp,"\n\n%d\n",q);
		}
		calculate(cz, fz, pz, rank, size, n, m, q);//рассчет 	
		MPI_Gather(fz,n*m/size, MPI_DOUBLE, zf, n*m/size, MPI_DOUBLE, 0, MPI_COMM_WORLD);//сбор однородных данных
		free(cz);//освобождение буферов
		free(pz);
		
		if (rank==0) {
			int count=0;

			memcpy(zp, zc, sizeof(double) * n * m);
			memcpy(zc, zf, sizeof(double) * n * m);
		
			for(double w = 0; w < height; w += dy) {
				for(double a = 0; a < width; a += dx) {
					fprintf(fp,"%f\t%f\t%f\n",a,w,zc[count]);
					count++;
				}
			}
		}
	}
	if(!rank) {
		gettimeofday(&stop,NULL); 
		timer = (stop.tv_sec - start.tv_sec) + (stop.tv_usec - start.tv_usec)/1000000.0; 
		printf("Time=%f\n",timer); 
		fclose(fp);
		fp = fopen("vgraph1.txt","w"); 
		fprintf(fp, "set dgrid3d %d %d \n", width, height);
		fprintf(fp, "set hidden3d\n");
		fprintf(fp,"set xrange[0:%d]\nset yrange[0:%d]\nset zrange[-5:5]\n", width, height);
	
		for(int i = 0;i < (int)(z/dt);i++) {
			fprintf(fp,"%s","splot 'vgraph.txt' ");
			fprintf(fp,"index %d using 1:2:3 with lines\n",i);
			fprintf(fp,"pause(0.01)\n");
		}
		fclose(fp); 
		MPI_Finalize();
	}
	system("gnuplot vgraph1.txt");
	return 0;
}
