#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define NUMBER_OF_TESTS 10

int main( argc, argv )
int argc;
char **argv;
{
    double       *buf;
    int          rank;
    int          n;
    double       t1, t2, tmin;
    int          j, k, nloop;
    MPI_Status   status;

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if (rank == 0) 
  printf( "Kind\t\tn\ttime (sec)\tRate (MB/sec)\n" );

    for (n=1; n<1100000; n*=2) {
  if (n == 0) nloop = 1000;
  else        nloop  = 1000/n;
  if (nloop < 1) nloop = 1;

  buf = (double *) malloc( n * sizeof(double) );
  if (!buf) {
      fprintf( stderr, 
         "Could not allocate send/recv buffer of size %d\n", n );
      MPI_Abort( MPI_COMM_WORLD, 1 );
  }
  tmin = 1000;
  for (k=0; k<NUMBER_OF_TESTS; k++) {
      if (rank == 0) {
    /* Make sure both processes are ready */
    MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, 1, 14,
            MPI_BOTTOM, 0, MPI_INT, 1, 14, MPI_COMM_WORLD, 
            &status );
    t1 = MPI_Wtime();
    for (j=0; j<nloop; j++) {
        MPI_Ssend( buf, n, MPI_DOUBLE, 1, k, MPI_COMM_WORLD );
        MPI_Recv( buf, n, MPI_DOUBLE, 1, k, MPI_COMM_WORLD, 
            &status );
    }
    t2 = (MPI_Wtime() - t1) / nloop;
    if (t2 < tmin) tmin = t2;
      }
      else if (rank == 1) {
    /* Make sure both processes are ready */
    MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, 0, 14,
            MPI_BOTTOM, 0, MPI_INT, 0, 14, MPI_COMM_WORLD, 
            &status );
    for (j=0; j<nloop; j++) {
        MPI_Recv( buf, n, MPI_DOUBLE, 0, k, MPI_COMM_WORLD, 
            &status );
        MPI_Ssend( buf, n, MPI_DOUBLE, 0, k, MPI_COMM_WORLD );
    }
      }
  }
  /* Convert to half the round-trip time */
  tmin = tmin / 2.0;
  if (rank == 0) {
      double rate;
      if (tmin > 0) rate = n * sizeof(double) * 1.0e-6 /tmin;
      else          rate = 0.0;
      printf( "Send/Recv\t%d\t%f\t%f\n", n, tmin, rate );
  }
  free( buf );
    }

    MPI_Finalize( );
    return 0;
}