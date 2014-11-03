/* reduce.c -- Parallel Trapezoidal Rule.  Uses 3 calls to MPI_Bcast to
 *    distribute input.  Also uses MPI_Reduce to compute final sum.
 *
 * Input: 
 *    a, b: limits of integration.
 *    n: number of trapezoids.
 * Output:  Estimate of the integral from a to b of f(x) 
 *    using the trapezoidal rule and n trapezoids.
 *
 * Notes:  
 *    1.  f(x) is hardwired.
 *    2.  the number of processes (p) should evenly divide
 *        the number of trapezoids (n).
 *
 * See Chap. 5, pp. 73 & ff. in PPMPI.
 */
#include <stdio.h>
#include <stdlib.h>

/* We'll be using MPI routines, definitions, etc. */
#include "mpi.h"

main(int argc, char** argv) {
    int         my_rank;   /* My process rank           */
    int         p;         /* The number of processes   */
    float       a;         /* Left endpoint             */
    float       b;         /* Right endpoint            */
    int         n;         /* Number of trapezoids      */
    float       h;         /* Trapezoid base length     */
    float       local_a;   /* Left endpoint my process  */
    float       local_b;   /* Right endpoint my process */
    int         local_n;   /* Number of trapezoids for  */
                           /* my calculation            */
    float       integral;  /* Integral over my interval */
    float       total;     /* Total integral            */
    int         source;    /* Process sending integral  */
    int         dest = 0;  /* All messages go to 0      */
    int         tag = 0;
    double 	t0,t1;	   /* Time variables */
    MPI_Status  status;
    
    void Get_data2(float*, float*, int*);
    
    /* Calculate local integral */
    float Trap(float local_a, float local_b, int local, float h);

    /* Let the system do what it needs to start up MPI */
    
    MPI_Init(&argc, &argv);
    t0 = MPI_Wtime();	
    /* Get my process rank */
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    /* Find out how many processes are being used */
    MPI_Comm_size(MPI_COMM_WORLD, &p);

   
    if (argc == 4){
		a = atof(argv[1]);
		b = atof(argv[2]);
		n = atoi(argv[3]);
    }else{
    if (my_rank == 0){
    	printf ("Usage: reduce A B N");
    	return(1);
    	}
    }
        
    Get_data2(&a, &b, &n);

    t0 = MPI_Wtime();
    
    h = (b-a)/n;    /* h is the same for all processes */
    local_n = n/p;  /* So is the number of trapezoids */

    /* Length of each process' interval of 
     * integration = local_n*h.  So my interval
     * starts at: */
    local_a = a + my_rank*local_n*h;
    local_b = local_a + local_n*h;
    integral = Trap(local_a, local_b, local_n, h);

    /* Add up the integrals calculated by each process */
    MPI_Reduce(&integral, &total, 1, MPI_FLOAT,
        MPI_SUM, 0, MPI_COMM_WORLD);

        /* Print the result */
    if (my_rank == 0) {
        printf("%d", 
            n);
        printf("%f, %f, %f,", 
            a, b, total);
    }

    /* Shut down MPI */
    MPI_Finalize();
       if (my_rank == 0) {
        t1 = MPI_Wtime() - t0;
        printf(" %f", t1);
}
} /*  main  */


/********************************************************************/
/* Function Get_data2
 * Reads in the user input a, b, and n.
 * Input parameters:
 *     1.  int my_rank:  rank of current process.
 *     2.  int p:  number of processes.
 * Output parameters:  
 *     1.  float* a_ptr:  pointer to left endpoint a.
 *     2.  float* b_ptr:  pointer to right endpoint b.
 *     3.  int* n_ptr:  pointer to number of trapezoids.
 * Algorithm:
 *     1.  Process 0 prompts user for input and
 *         reads in the values.
 *     2.  Process 0 sends input values to other
 *         processes using three calls to MPI_Bcast.
 */
void Get_data2(
         float*  a    /* in */, 
         float*  b    /* in */, 
         int*    n    /* in */) {

/*    float* a_ptr = a;
    float* b_ptr = b;
    int*   n_ptr = n;
*/    
    MPI_Bcast(a, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(b, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(n, 1, MPI_INT, 0, MPI_COMM_WORLD);
} /* Get_data2 */


/********************************************************************/
float Trap(
          float  local_a   /* in */, 
          float  local_b   /* in */, 
          int    local_n   /* in */, 
          float  h         /* in */) { 

    float integral;   /* Store result in integral  */ 
    float x; 
    int i; 

    float f(float x); /* function we're integrating */

    integral = (f(local_a) + f(local_b))/2.0; 
    x = local_a; 
    for (i = 1; i <= local_n-1; i++) { 
        x = x + h; 
        integral = integral + f(x); 
    } 
    integral = integral*h; 
    return integral;
} /*  Trap  */


/********************************************************************/
float f(float x) { 
    float return_val; 
    /* Calculate f(x). */
    /* Store calculation in return_val. */ 
    return_val = x*x;
    return return_val; 
} /* f */

