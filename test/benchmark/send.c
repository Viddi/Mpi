#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

#define NUMBER_OF_TESTS 10

/* 
   This version of pingpong sends and receives from the processes
   p, p + n/2
 */

int main( argc, argv )
int argc;
char **argv;
{
    double       *buf;
    int          rank, size;
    int          n;
    double       t1, t2, tmin, tmin_local;
    int          j, k, nloop;
    MPI_Status   status;
    int          source, dest;

    MPI_Init( &argc, &argv );

    MPI_Comm_rank( MPI_COMM_WORLD, &rank );
    MPI_Comm_size( MPI_COMM_WORLD, &size );
    if ((size % 2) != 0) {
  if (rank == 0) 
      printf( "Must use an even number of processes\n" );
  MPI_Abort( MPI_COMM_WORLD, 1 );
    }

    /* The rate printed by this program is the rate in the presense of contention */
    if (rank == 0) 
  printf( "Kind (np=%d)\tn\ttime (sec)\tRate (MB/sec)\n", size  );

    source = rank;
    dest   = (rank + (size/2)) % size;

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
      /* The barrier helps each link to start at about the same time */
      MPI_Barrier( MPI_COMM_WORLD );
      if (source < size/2) {
    /* Make sure both processes are ready */
    MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, dest, 14,
            MPI_BOTTOM, 0, MPI_INT, dest, 14, MPI_COMM_WORLD, 
            &status );
    t1 = MPI_Wtime();
    for (j=0; j<nloop; j++) {
        MPI_Ssend( buf, n, MPI_DOUBLE, dest, k, MPI_COMM_WORLD );
        MPI_Recv( buf, n, MPI_DOUBLE, dest, k, MPI_COMM_WORLD, 
            &status );
    }
    t2 = (MPI_Wtime() - t1) / nloop;
    if (t2 < tmin) tmin = t2;
      }
      else {
    tmin = 0.0;
    /* Make sure both processes are ready */
    MPI_Sendrecv( MPI_BOTTOM, 0, MPI_INT, dest, 14,
            MPI_BOTTOM, 0, MPI_INT, dest, 14, MPI_COMM_WORLD, 
            &status );
    for (j=0; j<nloop; j++) {
        MPI_Recv( buf, n, MPI_DOUBLE, dest, k, MPI_COMM_WORLD, 
            &status );
        MPI_Ssend( buf, n, MPI_DOUBLE, dest, k, MPI_COMM_WORLD );
    }
      }
  }
  /* Convert to half the round-trip time */
  tmin = tmin / 2.0;
  tmin_local = tmin;
  /* Get the WORST case for output (could use MPI_MAXLOC to get
     location as well) */
  MPI_Reduce( &tmin_local, &tmin, 1, MPI_DOUBLE, MPI_MAX, 0, 
           MPI_COMM_WORLD );
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