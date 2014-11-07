#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define NUMBER_OF_TESTS 10

int main( argc, argv )
int argc;
char **argv;
{
    double       *sbuf, *rbuf;
    int          rank;
    int          n;
    double       t1, t2, tmin;
    int          i, j, k, nloop;
    MPI_Status   status, statuses[2];
    MPI_Request  r[2];

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    if (rank == 0) 
  printf( "Kind\t\t\t\tn\ttime (sec)\tRate (MB/sec)\n" );

    for (n=1; n<1100000; n*=2) {
  if (n == 0) nloop = 1000;
  else        nloop  = 1000/n;
  if (nloop < 1) nloop = 1;

  sbuf = (double *) malloc( n * sizeof(double) );
  rbuf = (double *) malloc( n * sizeof(double) );
  if (!sbuf || !rbuf) {
      fprintf( stderr, 
         "Could not allocate send/recv buffers of size %d\n", n );
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
        MPI_Isend( sbuf, n, MPI_DOUBLE, 1, k, MPI_COMM_WORLD, 
             &r[0] );
        MPI_Irecv( rbuf, n, MPI_DOUBLE, 1, k, MPI_COMM_WORLD, 
             &r[1] );
        MPI_Waitall( 2, r, statuses );
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
        MPI_Irecv( rbuf, n, MPI_DOUBLE, 0, k, MPI_COMM_WORLD, 
            &r[0] );
        MPI_Isend( sbuf, n, MPI_DOUBLE, 0, k, MPI_COMM_WORLD, 
             &r[1] );
        MPI_Waitall( 2, r, statuses );
    }
      }
  }

  /* rate is MB/sec for exchange, not an estimate of the 
     component for each isend/irecv */
  if (rank == 0) {
      double rate;
      if (tmin > 0) rate = 2 * n * sizeof(double) * 1.0e-6 /tmin;
      else          rate = 0.0;
      printf( "head-to-head Isend/Irecv\t%d\t%f\t%f\n", n, tmin, rate );
  }
  free( sbuf ); free( rbuf );
    }

    MPI_Finalize( );
    return 0;
}
