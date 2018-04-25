#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#define _REENTRANT
#include <mpi.h>
#include <sched.h>

int N = 0;
int M = 0;
double dx = 0;
double dy = 0;
double dt = 0;
double t = 0;
double a = 1;

int time_steps = 0;

struct timeval tv1, tv2, dtv;
struct timezone tz;


void time_start() {
    gettimeofday(&tv1, &tz);
}

int boundary_conditions() {
    return 0; // граничное условие первого рода
}

void * explicit_method(double *prev_iter, double *curr_iter, double *next_iter, int num, double *recv_buf_left, double *recv_buf_right, int proc_id, int current_time) {
    double z_11, z_21, z_01, z_12, z_10, z_11_0, f;
    int i = 0;

    for (i = 0; i < num; i++) {
        if ((i < N && recv_buf_left == NULL) || i % N == 0 || i % N == N - 1 || (i > num - N  && recv_buf_right == NULL)) {
            next_iter[i] = boundary_conditions();
        } else {
            z_11 = curr_iter[i]; // z_i_j t = 0
            z_21 = curr_iter[i + 1]; // z_i+1_j t = 0
            z_01 = curr_iter[i - 1]; // z_i-1_j t = 0

            if (i + N > num) {
                z_12 = recv_buf_right[i % N];
            } else {
                z_12 = curr_iter[i + N]; // z_i+1_j t = 0
            }

            if (i - N < 0) {
                z_10 = recv_buf_left[i % N];
            } else {
                z_10 = curr_iter[i - N]; // z_i-1_j t = 0
            }

            z_11_0 = prev_iter[i]; // z_i_j t = -1
            f=0;
            if (current_time < time_steps / 2) {
                 f = 6 ; /// dt * sin((i % N) * (i / N) * t; // f(x,y,t)
                 //printf("%d %d\n", current_time, time_steps/4);
            }
           

            next_iter[i] = (a * a * dt * dt / (dx * dx)) * (z_21 + z_01 + z_12 + z_10 - 4 * z_11) + dt * dt * f - z_11_0 + 2 * z_11;
        }
    }
}
int time_stop() {
    gettimeofday(&tv2, &tz);
    dtv.tv_sec = tv2.tv_sec - tv1.tv_sec;
    dtv.tv_usec = tv2.tv_usec - tv1.tv_usec;

    if (dtv.tv_usec < 0) {
        dtv.tv_sec--;
        dtv.tv_usec += 1000000;
    }

    return dtv.tv_sec * 1000 + dtv.tv_usec / 1000;
}

void * add_new_iter(FILE * input_file, double *res_curr_iter, int num_points, int num) {
    int k_1 = 0;
    int j = 0;

    for (j = 0; j < num_points; j++) {
        int k = j % num;

        fprintf (input_file, "%lf ", res_curr_iter[k * N + k_1]);

        if (k == num - 1) {
            fprintf(input_file, "\n");
            // printf("\n");
            k_1++;
        }
    }

    // printf("\n\n");
    fprintf(input_file, "\n\n");
}

void * visualize(FILE *input_file, int num_time) {
    int i;
    FILE *gnu = popen("gnuplot -persist", "w");
    fprintf (gnu, "set zrange [-1:1]\n");

    for (i = 1; i < num_time; i++) {
        fprintf (gnu, "splot \"input.txt\" index %d matrix w l\n", i);
        fprintf (gnu, "pause 0.1\n");
    }

    fflush  (gnu);
}


int main(int argc, char *argv[]) {
    int i = 0, j = 0, t_out = 100, offset = 0;
    double *res_curr_iter, *res_prev_iter, *next_iter,  *curr_iter, *prev_iter, *save_iter, *save_res, *recv_buf_left, *recv_buf_right;
    FILE *input_file = NULL;

    int num_time = 0;

    double height = 1;
    double width = 1;

    int num_points = 0;
    int num_proc = 0;
    int num_intervals = 0;
    int proc_id = 0;
    MPI_Status status;

    if (argc < 4) {
        printf("Input: #points  #iterations  #threads\n");
        exit (1);
    }

    num_points = atoi(argv[1]);
    time_steps = atoi(argv[2]);

    double num = (-(width / height + 1) + sqrt(pow(width / height + 1, 2) - 4 * (width / height) * (1 - num_points))) / (2 * width / height); // (-(w / h + 1) + sqrt(sqr(w / h + 1) - 4 * (w / h) * (1 - k))) / (2 * (w / h));
    num_intervals = (int) num;
    input_file = fopen("input.txt", "w");
    time_start();
    // инциализая MPI, рутового процесса и потомков + определение номера каждого процесса и кол-ва этих процессов
    // инициализация переменной num_proc

    MPI_Init (&argc, &argv); //инициализация
    MPI_Comm_size (MPI_COMM_WORLD, &num_proc); //всего процессов, получение
    MPI_Comm_rank (MPI_COMM_WORLD, &proc_id); //получение номера процесса

    if (num_intervals != num || ((int) (num_intervals * width / height) + 1) % num_proc != 0) {
        printf("Error! Number of point can not be divident (?) exactly by number of threads\n");
        MPI_Finalize();
        exit (1);
    }

    N = num_intervals + 1; // vertical dimention
    M = num_intervals * (width / height) + 1; //// horizontal dimention
    dx = dy = height / num_intervals;
    dt = dx * dx / (2 * a);
    offset = (num_intervals * width / height + 1) / num_proc; // ширина полосы для каждого процесса

    if (!proc_id) {
        res_curr_iter = (double *) calloc (num_points, sizeof(double));
        res_prev_iter = (double *) calloc (num_points, sizeof(double));
    }

    next_iter = (double *) calloc (offset * N, sizeof(double));
    curr_iter = (double *) calloc (offset * N, sizeof(double));
    prev_iter = (double *) calloc (offset * N, sizeof(double));

    MPI_Scatter ( res_curr_iter, offset * N, MPI_DOUBLE, curr_iter, offset * N, MPI_DOUBLE, 0, MPI_COMM_WORLD); 
    MPI_Scatter ( res_prev_iter, offset * N, MPI_DOUBLE, prev_iter, offset * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    recv_buf_left = (double *) calloc (N, sizeof(double));
    recv_buf_right = (double *) calloc (N, sizeof(double));

    for(i = 0; i < time_steps; i++) {
        if (proc_id != 0) {
            MPI_Sendrecv( curr_iter, N, MPI_DOUBLE, proc_id - 1, 0, recv_buf_left, N, MPI_DOUBLE, proc_id - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status); 
        } else {
            recv_buf_left = NULL;
        }

        if (proc_id != num_proc - 1) {
            MPI_Sendrecv( &curr_iter[offset * N - N], N, MPI_DOUBLE, proc_id + 1, 1, recv_buf_right, N, MPI_DOUBLE, proc_id + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        } else {
            recv_buf_right = NULL;
        }

        explicit_method(prev_iter, curr_iter, next_iter, offset * N, recv_buf_left, recv_buf_right, proc_id, i); 

        save_iter = prev_iter;
        prev_iter = curr_iter;
        curr_iter = next_iter;
        next_iter = save_iter;

        if (!proc_id) {
            save_res = res_prev_iter;
            res_prev_iter = res_curr_iter;
            res_curr_iter = save_res;
        }

        MPI_Gather (curr_iter, offset * N, MPI_DOUBLE, res_curr_iter, offset * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (!proc_id) {
            num_time++;
            add_new_iter(input_file, res_curr_iter, num_points, N);
        }

        t += dt;
    }

    MPI_Barrier (MPI_COMM_WORLD);
    MPI_Finalize();

    int ms = time_stop();

    if (!proc_id) {
        visualize(input_file, num_time);
        printf("Time: %d milliseconds\n", ms);
        fclose(input_file);
        free(recv_buf_left);
        free(recv_buf_right);
        free(res_curr_iter);
        free(res_prev_iter);
        free(next_iter);
        free(curr_iter);
        free(prev_iter);
    }

    exit (0);
}