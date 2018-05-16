#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <mpi.h>
#define _REENTRANT

#define APROX_NUM_OF_NODES_X 10240000
#define TIME_COUNT 1000

int main (int argc, char** argv) {
    int numOfNodes = APROX_NUM_OF_NODES_X;
    int myrank, total, nodesPeerProc;
    double *AllNodes, *nodesT1, *nodesT2;
    int granUsl, length;
    int intBuf[1];
    int timeIterCount = TIME_COUNT;
    double doubleBuf[2];
    double hx, ht, alpha;
    double node_l, node_r;
    MPI_Status mpi_status;
    double leftTemp, rightTemp, lQstream, rQstream, Tstart;
    FILE * gnuplotPipe;

    MPI_Init (&argc, &argv);
    MPI_Comm_size (MPI_COMM_WORLD, &total);
    MPI_Comm_rank (MPI_COMM_WORLD, &myrank);
    //printf ("Total=%d, rank=%d\n", total, myrank);

    numOfNodes -= numOfNodes % total;

    if (!myrank) {
//        printf("Enter the length:\n");
//        scanf("%d", &length);
length = 1;
//        printf("Enter ALPHA:\n");
//        scanf("%lf", &alpha);
alpha = 1;
//        printf("Enter boundary condition type (1 or 2):\n");
//        scanf("%d", &granUsl);
granUsl = 1;
//        if(granUsl != 1 && granUsl != 2) {
//            printf("Incorrect boundary condition type.\n");
//            MPI_Abort (MPI_COMM_WORLD, MPI_ERR_OTHER);
//        }

//        if (granUsl == 1) {
//            printf("Enter left temperature:\n");
//            scanf("%lf", &leftTemp);
//            printf("Enter right temperature:\n");
//            scanf("%lf", &rightTemp);
//        }
leftTemp = 50;
rightTemp = 40;
/*        if (granUsl == 2) {
            printf("Enter left Qsrteam:\n");
            scanf("%lf", &lQstream);
            printf("Enter right Qsrteam:\n");
            scanf("%lf", &rQstream);
        }
*/
        AllNodes = (double *) malloc (sizeof(double) * numOfNodes);
        
//        printf("T(t = 0) = ");
//        scanf("%lf", &Tstart);
Tstart = 20;
        for (int i = 1; i < numOfNodes - 1; i++) {
            AllNodes[i] = Tstart;
        }
        if (granUsl == 1) {
            AllNodes[0] = leftTemp;
            AllNodes[numOfNodes - 1] = rightTemp;
        }
        if (granUsl == 2) {
            int delta = (double)length / (numOfNodes - 1);
            AllNodes[0] = lQstream * delta + AllNodes[1];
            AllNodes[numOfNodes - 1] = AllNodes[numOfNodes - 2] - rQstream * delta;
        }
        
    }
    if (!myrank) {  
        intBuf[0] = numOfNodes / total;
        doubleBuf[0] = (double)length / (numOfNodes - 1);
        doubleBuf[1] = alpha;        
    };
    
    MPI_Bcast((void *)intBuf, 1, MPI_INT, 0, MPI_COMM_WORLD);
    nodesPeerProc = intBuf[0];
    MPI_Bcast((void *)doubleBuf, 2, MPI_DOUBLE, 0, MPI_COMM_WORLD);    
    hx = doubleBuf[0];
    alpha = doubleBuf[1];
    ht = hx * hx / (2 * alpha);
        

    nodesT1 = (double *) malloc (sizeof(double) * nodesPeerProc);
    nodesT2 = (double *) malloc (sizeof(double) * nodesPeerProc);

    // if (!myrank) {
    //     for (int i = 0; i < numOfNodes; i++) {
    //         printf("%lf ", AllNodes[i]);
    //     }
    //     printf("\n\n");
    // }
/*
    if (!myrank) {
        gnuplotPipe = popen("gnuplot -persistent", "w");
        fprintf(gnuplotPipe, "set terminal gif animate delay 5\n");
        fprintf(gnuplotPipe, "set output 'animate.gif'\n");
        fprintf(gnuplotPipe, "set xlabel 'Length'\n");
        fprintf(gnuplotPipe, "set ylabel 'Temperature'\n"); 
        
        
        // fprintf(gnuplotPipe, "set ticslevel 0\n");
        //fprintf(gnuplotPipe, "set dgrid3d 40, 40 splines\n");
        fprintf(gnuplotPipe, "set xrange[0:%d]\n", length);
        fprintf(gnuplotPipe, "do for [i=1:%d] {\n", timeIterCount);       
        fprintf(gnuplotPipe, "plot '-' with lines lw 4 title 'Temp'\n");
        fprintf(gnuplotPipe, "}\n");
    }
*/
MPI_Scatter((void *)AllNodes, nodesPeerProc, MPI_DOUBLE, (void *)nodesT1, nodesPeerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    for(int k = 0; k < timeIterCount; k++) {
        
       // MPI_Scatter((void *)AllNodes, nodesPeerProc, MPI_DOUBLE, (void *)nodesT1, nodesPeerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

for(int q = 0; q < nodesPeerProc; q++) {
    nodesT1[q] = nodesT2[q];
}
        if (total > 1 && (myrank < total - 1)) {
            MPI_Send ((nodesT1 + nodesPeerProc - 1), 1, MPI_DOUBLE, myrank + 1, 0, MPI_COMM_WORLD);
        }
        if (total > 1 && myrank) {
            MPI_Recv (&node_l, 1, MPI_DOUBLE, myrank - 1, 0, MPI_COMM_WORLD, &mpi_status);
        }
        if (total > 1 && myrank) {
            MPI_Send ((nodesT1), 1, MPI_DOUBLE, myrank - 1, 0, MPI_COMM_WORLD);
        }
        if (total > 1 && (myrank < total - 1)) {
            MPI_Recv (&node_r, 1, MPI_DOUBLE, myrank + 1, 0, MPI_COMM_WORLD, &mpi_status);
        }

        if (myrank && (myrank < total - 1)) {
            nodesT2[0] = alpha * (nodesT1[1] - 2 * nodesT1[0] + node_l) * (ht/(hx * hx)) + nodesT1[0];
            nodesT2[nodesPeerProc - 1] = alpha * (node_r - 2 * nodesT1[nodesPeerProc - 1] + nodesT1[nodesPeerProc - 2]) * (ht/(hx * hx)) + nodesT1[nodesPeerProc - 1];
        }
        if (total > 1 && !myrank) {
            nodesT2[nodesPeerProc - 1] = alpha * (node_r - 2 * nodesT1[nodesPeerProc - 1] + nodesT1[nodesPeerProc - 2]) * (ht/(hx * hx)) + nodesT1[nodesPeerProc - 1];
        }
        if (total > 1 && (myrank == total - 1)) {
            nodesT2[0] = alpha * (nodesT1[1] - 2 * nodesT1[0] + node_l) * (ht/(hx * hx)) + nodesT1[0];
        }

        for (int i = 1; i < nodesPeerProc - 1; i++) {
            nodesT2[i] = alpha * (nodesT1[i + 1] - 2 * nodesT1[i] + nodesT1[i - 1]) * (ht/(hx * hx)) + nodesT1[i];
        }

//        MPI_Gather((void *)nodesT2, nodesPeerProc, MPI_DOUBLE, (void *)AllNodes, nodesPeerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        if (!myrank) {
            nodesT2[0] = leftTemp;
            //AllNodes[numOfNodes - 1] = rightTemp;
        }
        if ((myrank == total - 1)) {
            //AllNodes[0] = lQstream * hx + AllNodes[1];
            //AllNodes[numOfNodes - 1] = AllNodes[numOfNodes - 2] - rQstream * hx;
            nodesT2[nodesPeerProc - 1] = rightTemp;
        }
/*
        if (!myrank) {
            double xCoor = 0;
            for (int i = 0; i < numOfNodes; i++) {
                fprintf(gnuplotPipe, "%lf %lf\n", xCoor, AllNodes[i]);
                xCoor += hx;
            }
            fprintf(gnuplotPipe, "e\n");
        }
*/
        // if (!myrank && k % 100000 == 0) {
        //     for (int i = 0; i < numOfNodes; i++) {
        //         printf("%lf ", AllNodes[i]);
        //     }
        //     printf("\n\n\n");
        // }
    }

 MPI_Gather((void *)nodesT2, nodesPeerProc, MPI_DOUBLE, (void *)AllNodes, nodesPeerProc, MPI_DOUBLE, 0, MPI_COMM_WORLD);


    free(nodesT1);
    free(nodesT2);
    if (!myrank) {
//        pclose(gnuplotPipe);
        free(AllNodes);        
    }
    MPI_Finalize();    
    exit(0);
}
