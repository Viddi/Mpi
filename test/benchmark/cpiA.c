/* cpi.c -- calculates pi
 *
 * Input:
 *     n: number of iterations
 * Output:
 *     p: calculated pi
 *     e: percent error of accepted value of pi
 *     t: processing time.
 *
 */

#include "mpi.h"
#include <stdio.h>
#include <math.h>

int main( int argc, char *argv[] )
{
    int n, myid, numprocs; 
    long i;
    double PI25DT = 3.141592653589793238462643;
    double mypi, pi, h, sum, x;
    double t0,t1;

    //Initialize MPI environment
    MPI_Init(&argc,&argv);
    t0 = MPI_Wtime(); 
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);

    //Accept first command line argument
    n = atol(argv[1]);
         
    //Send argument to all node in cluster
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
               
    h   = 1.0 / (double) n;
    sum = 0.0;
    //Calculates pi
    for (i = myid + 1; i <= n; i += numprocs) {
        x = h * ((double)i - 0.5);
        sum += (4.0 / (1.0 + x*x));
    }
    mypi = h * sum;
    //Reduce result
    MPI_Reduce(&mypi, &pi, 1, MPI_DOUBLE, MPI_SUM, 0,
    			MPI_COMM_WORLD);
    //Print results
    if (myid == 0) {
        t1 = MPI_Wtime() - t0;
        printf("pi is approximately %.16f, Error is %.16f\n",
               pi, fabs(pi - PI25DT));
                 
    }

    //Close mpi environment and stop timer
    MPI_Finalize();
       if (myid == 0) {
               t1 = MPI_Wtime() - t0;
               printf("Time: %f\n", t1);
       }    
    return 0;
}

