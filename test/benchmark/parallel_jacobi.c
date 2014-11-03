/* parallel_jacobi.c -- parallel implementation of Jacobi's method
 *     for solving the linear system Ax = b.  Uses block distribution
 *     of vectors and block-row distribution of A.
 * 
 * Input:
 *     n:  order of system
 *     tol:  convergence tolerance
 *     max_iter:  maximum number of iterations
 *     A:  coefficient matrix
 *     b:  right-hand side of system
 *
 * Output:
 *     x:  the solution if the method converges
 *     max_iter:  if the method fails to converge
 *
 * Notes:  
 *     1.  A should be strongly diagonally dominant in
 *         order to insure convergence.
 *     2.  A, x, and b are statically allocated.
 *
 * See Chap 10, pp. 220 & ff in PPMPI.
 */
#include <stdio.h>
#include "mpi.h"
#include <math.h>

#define Swap(x,y) {float* temp; temp = x; x = y; y = temp;}

#define MAX_DIM 12

typedef float MATRIX_T[MAX_DIM][MAX_DIM];

int Parallel_jacobi(
        MATRIX_T  A_local    /* in  */, 
        float     x_local[]  /* out */, 
        float     b_local[]  /* in  */, 
        int       n          /* in  */, 
        float     tol        /* in  */, 
        int       max_iter   /* in  */,
        int       p          /* in  */, 
        int       my_rank    /* in  */);

void Read_matrix(char* prompt, MATRIX_T A_local, int n,
         int my_rank, int p);
void Read_vector(char* prompt, float x_local[], int n, int my_rank,
         int p);
void Print_matrix(char* title, MATRIX_T A_local, int n, 
         int my_rank, int p);
void Print_vector(char* title, float x_local[], int n, int my_rank,
         int p);

main(int argc, char* argv[]) {
    int        p;
    int        my_rank;
    MATRIX_T   A_local;
    float      x_local[MAX_DIM];
    float      b_local[MAX_DIM];
    int        n;
    float      tol;
    int        max_iter;
    int        converged;
    double     t0, t1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (my_rank == 0) {
        printf("Enter n, tolerance, and max number of iterations\n");
        scanf("%d %f %d", &n, &tol, &max_iter);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&tol, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&max_iter, 1, MPI_INT, 0, MPI_COMM_WORLD);

    Read_matrix("Enter the matrix", A_local, n, my_rank, p);
    Read_vector("Enter the right-hand side", b_local, n, my_rank, p);

    t0 = MPI_Wtime();

    converged = Parallel_jacobi(A_local, x_local, b_local, n,
        tol, max_iter, p, my_rank);

    t1 = MPI_Wtime() - t0;

    if (converged){
        Print_vector("The solution is\n", x_local, n, my_rank, p);
        printf("Total execution time: %f\n", t1);
    }
    else
        if (my_rank == 0)
            printf("Failed to converge in %d iterations\n", max_iter);

    MPI_Finalize();
}  /* main */


/*********************************************************************/
/* Return 1 if iteration converged, 0 otherwise */
/* MATRIX_T is a 2-dimensional array            */
int Parallel_jacobi(
        MATRIX_T  A_local    /* in  */, 
        float     x_local[]  /* out */, 
        float     b_local[]  /* in  */, 
        int       n          /* in  */, 
        float     tol        /* in  */, 
        int       max_iter   /* in  */,
        int       p          /* in  */, 
        int       my_rank    /* in  */) {
    int     i_local, i_global, j;
    int     n_bar;
    int     iter_num;
    float   x_temp1[MAX_DIM];
    float   x_temp2[MAX_DIM];
    float*  x_old;
    float*  x_new;

    float Distance(float x[], float y[], int n);

    n_bar = n/p;
    
    /* Initialize x */
    MPI_Allgather(b_local, n_bar, MPI_FLOAT, x_temp1,
        n_bar, MPI_FLOAT, MPI_COMM_WORLD);
    x_new = x_temp1;
    x_old = x_temp2;

    iter_num = 0;
    do {
        iter_num++;
        
        /* Interchange x_old and x_new */
        Swap(x_old, x_new);
        for (i_local = 0; i_local < n_bar; i_local++){
            i_global = i_local + my_rank*n_bar;
            x_local[i_local] = b_local[i_local];
            for (j = 0; j < i_global; j++)
                x_local[i_local] = x_local[i_local] -  
                    A_local[i_local][j]*x_old[j];
            for (j = i_global+1; j < n; j++)
                x_local[i_local] = x_local[i_local] -   
                    A_local[i_local][j]*x_old[j];
            x_local[i_local] = x_local[i_local]/
                    A_local[i_local][i_global];
        }

        MPI_Allgather(x_local, n_bar, MPI_FLOAT, x_new,
            n_bar, MPI_FLOAT, MPI_COMM_WORLD);
    } while ((iter_num < max_iter) && 
             (Distance(x_new,x_old,n) >= tol));

    if (Distance(x_new,x_old,n) < tol)
        return 1;
    else
        return 0;
} /* Jacobi */


/*********************************************************************/
float Distance(float x[], float y[], int n) {
    int i;
    float sum = 0.0;

    for (i = 0; i < n; i++) {
        sum = sum + (x[i] - y[i])*(x[i] - y[i]);
    }
    return sqrt(sum);
} /* Distance */


/*********************************************************************/
void Read_matrix(
         char*     prompt   /* in  */,
         MATRIX_T  A_local  /* out */,
         int       n        /* in  */,
         int       my_rank  /* in  */,
         int       p        /* in  */) {

    int       i, j;
    MATRIX_T  temp;
    int       n_bar;
 
    n_bar = n/p;

    /* Fill dummy entries in temp with zeroes */
    for (i = 0; i < n; i++)
        for (j = n; j < MAX_DIM; j++)
            temp[i][j] = 0.0;

    if (my_rank == 0) {
        printf("%s\n", prompt);
        for (i = 0; i < n; i++)
            for (j = 0; j < n; j++)
                scanf("%f",&temp[i][j]);
    }
    MPI_Scatter(temp, n_bar*MAX_DIM, MPI_FLOAT, A_local,
        n_bar*MAX_DIM, MPI_FLOAT, 0, MPI_COMM_WORLD);

}  /* Read_matrix */

/*********************************************************************/
void Read_vector(
         char*  prompt     /* in  */,
         float  x_local[]  /* out */,
         int    n          /* in  */,
         int    my_rank    /* in  */,
         int    p          /* in  */) {

    int   i;
    float temp[MAX_DIM];
    int   n_bar;
    
    n_bar = n/p;

    if (my_rank == 0) {
        printf("%s\n", prompt);
        for (i = 0; i < n; i++)
            scanf("%f", &temp[i]);
    }
    MPI_Scatter(temp, n_bar, MPI_FLOAT, x_local, n_bar, MPI_FLOAT,
        0, MPI_COMM_WORLD);

}  /* Read_vector */


/*********************************************************************/
void Print_matrix(char* title, MATRIX_T A_local, int n, 
         int my_rank, int p);
void Print_matrix(
         char*     title      /* in */,
         MATRIX_T  A_local    /* in */,
         int       n          /* in */,
         int       my_rank    /* in */,
         int       p          /* in */) {

    int       i, j;
    MATRIX_T  temp;
    int       n_bar;

    n_bar = n/p;

    MPI_Gather(A_local, n_bar*MAX_DIM, MPI_FLOAT, temp,
         n_bar*MAX_DIM, MPI_FLOAT, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("%s\n", title);
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++)
                printf("%4.1f ", temp[i][j]);
            printf("\n");
        }
    }
}  /* Print_matrix */


/*********************************************************************/
void Print_vector(char* title, float x_local[], int n, int my_rank,
         int p);
void Print_vector(
         char*  title      /* in */,
         float  x_local[]  /* in */,
         int    n          /* in */,
         int    my_rank    /* in */,
         int    p          /* in */) {

    int   i;
    float temp[MAX_DIM];
    int   n_bar;

    n_bar = n/p;

    MPI_Gather(x_local, n_bar, MPI_FLOAT, temp, n_bar, MPI_FLOAT,
        0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        printf("%s\n", title);
        for (i = 0; i < n; i++)
            printf("%4.1f ", temp[i]);
        printf("\n");
    }
}  /* Print_vector */
