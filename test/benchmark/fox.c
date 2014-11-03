/* fox.c -- uses Fox's algorithm to multiply two square matrices
 *
 * Input:
 *     n: global order of matrices
 *     A,B: the factor matrices
 * Output:
 *     C: the product matrix
 *
 * Notes:  
 *     1.  Assumes the number of processes is a perfect square
 *     2.  The array member of the matrices is statically allocated
 *     3.  Assumes the global order of the matrices is evenly
 *         divisible by sqrt(p).
 *
 * See Chap 7, pp. 113 & ff and pp. 125 & ff in PPMPI
 */
#include <stdio.h>
#include "mpi.h"
#include <math.h>
#include <stdlib.h>

typedef struct {
    int       p;         /* Total number of processes    */
    MPI_Comm  comm;      /* Communicator for entire grid */
    MPI_Comm  row_comm;  /* Communicator for my row      */
    MPI_Comm  col_comm;  /* Communicator for my col      */
    int       q;         /* Order of grid                */
    int       my_row;    /* My row number                */
    int       my_col;    /* My column number             */
    int       my_rank;   /* My rank in the grid comm     */
} GRID_INFO_T;


#define MAX 65536
typedef struct {
    int     n_bar;
#define Order(A) ((A)->n_bar)
    float  entries[MAX];
#define Entry(A,i,j) (*(((A)->entries) + ((A)->n_bar)*(i) + (j)))
} LOCAL_MATRIX_T;

/* Function Declarations */
LOCAL_MATRIX_T*  Local_matrix_allocate(int n_bar);
void             Free_local_matrix(LOCAL_MATRIX_T** local_A);
void             Read_matrix(char* prompt, LOCAL_MATRIX_T* local_A, 
                     GRID_INFO_T* grid, int n);
void             Print_matrix(char* title, LOCAL_MATRIX_T* local_A, 
                     GRID_INFO_T* grid, int n);
void             Set_to_zero(LOCAL_MATRIX_T* local_A);
void             Local_matrix_multiply(LOCAL_MATRIX_T* local_A,
                     LOCAL_MATRIX_T* local_B, LOCAL_MATRIX_T* local_C);
void             Build_matrix_type(LOCAL_MATRIX_T* local_A);
MPI_Datatype     local_matrix_mpi_t;

LOCAL_MATRIX_T*  temp_mat;
void             Print_local_matrices(char* title, LOCAL_MATRIX_T* local_A, 
                     GRID_INFO_T* grid);

/*********************************************************/
main(int argc, char* argv[]) {
    int              p;
    int              my_rank;
    GRID_INFO_T      grid;
    LOCAL_MATRIX_T*  local_A;
    LOCAL_MATRIX_T*  local_B;
    LOCAL_MATRIX_T*  local_C;
    int              n;
    int              n_bar;
    double 	     t1, t0;

    void Setup_grid(GRID_INFO_T*  grid);
    void Fox(int n, GRID_INFO_T* grid, LOCAL_MATRIX_T* local_A,
             LOCAL_MATRIX_T* local_B, LOCAL_MATRIX_T* local_C);

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    Setup_grid(&grid);
    if (my_rank == 0) {
        printf("What's the order of the matrices?\n");
        scanf("%d", &n);
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    n_bar = n/grid.q;

    local_A = Local_matrix_allocate(n_bar);
    Order(local_A) = n_bar;
    Read_matrix("Enter A", local_A, &grid, n);
    Print_matrix("We read A =", local_A, &grid, n);

    local_B = Local_matrix_allocate(n_bar);
    Order(local_B) = n_bar;
    Read_matrix("Enter B", local_B, &grid, n);
    Print_matrix("We read B =", local_B, &grid, n);

    t0 = MPI_Wtime();	
	
    Build_matrix_type(local_A);
    temp_mat = Local_matrix_allocate(n_bar);

    local_C = Local_matrix_allocate(n_bar);
    Order(local_C) = n_bar;
    Fox(n, &grid, local_A, local_B, local_C);

    t1 = MPI_Wtime() - t0;

    Print_matrix("The product is", local_C, &grid, n); 

    if (my_rank == 0)        
        printf("Execution time: %f\n",t1);

    Free_local_matrix(&local_A);
    Free_local_matrix(&local_B);
    Free_local_matrix(&local_C);

    MPI_Finalize();
}  /* main */


/*********************************************************/
void Setup_grid(
         GRID_INFO_T*  grid  /* out */) {
    int old_rank;
    int dimensions[2];
    int wrap_around[2];
    int coordinates[2];
    int free_coords[2];

    /* Set up Global Grid Information */
    MPI_Comm_size(MPI_COMM_WORLD, &(grid->p));
    MPI_Comm_rank(MPI_COMM_WORLD, &old_rank);

    /* We assume p is a perfect square */
    grid->q = (int) sqrt((double) grid->p);
    dimensions[0] = dimensions[1] = grid->q;

    /* We want a circular shift in second dimension. */
    /* Don't care about first                        */
    wrap_around[0] = wrap_around[1] = 1;
    MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, 
        wrap_around, 1, &(grid->comm));
    MPI_Comm_rank(grid->comm, &(grid->my_rank));
    MPI_Cart_coords(grid->comm, grid->my_rank, 2, 
        coordinates);
    grid->my_row = coordinates[0];
    grid->my_col = coordinates[1];

    /* Set up row communicators */
    free_coords[0] = 0; 
    free_coords[1] = 1;
    MPI_Cart_sub(grid->comm, free_coords, 
        &(grid->row_comm));

    /* Set up column communicators */
    free_coords[0] = 1; 
    free_coords[1] = 0;
    MPI_Cart_sub(grid->comm, free_coords, 
        &(grid->col_comm));
} /* Setup_grid */


/*********************************************************/
void Fox(
        int              n         /* in  */, 
        GRID_INFO_T*     grid      /* in  */, 
        LOCAL_MATRIX_T*  local_A   /* in  */,
        LOCAL_MATRIX_T*  local_B   /* in  */,
        LOCAL_MATRIX_T*  local_C   /* out */) {

    LOCAL_MATRIX_T*  temp_A; /* Storage for the sub-    */
                             /* matrix of A used during */ 
                             /* the current stage       */
    int              stage;
    int              bcast_root;
    int              n_bar;  /* n/sqrt(p)               */
    int              source;
    int              dest;
    MPI_Status       status;

    n_bar = n/grid->q;
    Set_to_zero(local_C);

    /* Calculate addresses for circular shift of B */  
    source = (grid->my_row + 1) % grid->q;
    dest = (grid->my_row + grid->q - 1) % grid->q;

    /* Set aside storage for the broadcast block of A */
    temp_A = Local_matrix_allocate(n_bar);

    for (stage = 0; stage < grid->q; stage++) {
        bcast_root = (grid->my_row + stage) % grid->q;
        if (bcast_root == grid->my_col) {
            MPI_Bcast(local_A, 1, local_matrix_mpi_t,
                bcast_root, grid->row_comm);
            Local_matrix_multiply(local_A, local_B, 
                local_C);
        } else {
            MPI_Bcast(temp_A, 1, local_matrix_mpi_t,
                bcast_root, grid->row_comm);
            Local_matrix_multiply(temp_A, local_B, 
                local_C);
        }
        MPI_Sendrecv_replace(local_B, 1, local_matrix_mpi_t,
            dest, 0, source, 0, grid->col_comm, &status);
    } /* for */
    
} /* Fox */


/*********************************************************/
LOCAL_MATRIX_T* Local_matrix_allocate(int local_order) {
    LOCAL_MATRIX_T* temp;
  
    temp = (LOCAL_MATRIX_T*) malloc(sizeof(LOCAL_MATRIX_T));
    return temp;
}  /* Local_matrix_allocate */


/*********************************************************/
void Free_local_matrix(
         LOCAL_MATRIX_T** local_A_ptr  /* in/out */) {
    free(*local_A_ptr);
}  /* Free_local_matrix */


/*********************************************************/
/* Read and distribute matrix:  
 *     foreach global row of the matrix,
 *         foreach grid column 
 *             read a block of n_bar floats on process 0
 *             and send them to the appropriate process.
 */
void Read_matrix(
         char*            prompt   /* in  */, 
         LOCAL_MATRIX_T*  local_A  /* out */,
         GRID_INFO_T*     grid     /* in  */,
         int              n        /* in  */) {

    int        mat_row, mat_col;
    int        grid_row, grid_col;
    int        dest;
    int        coords[2];
    float*     temp;
    MPI_Status status;
    
    if (grid->my_rank == 0) {
        temp = (float*) malloc(Order(local_A)*sizeof(float));
        printf("%s\n", prompt);
        fflush(stdout);
        for (mat_row = 0;  mat_row < n; mat_row++) {
            grid_row = mat_row/Order(local_A);
            coords[0] = grid_row;
            for (grid_col = 0; grid_col < grid->q; grid_col++) {
                coords[1] = grid_col;
                MPI_Cart_rank(grid->comm, coords, &dest);
                if (dest == 0) {
                    for (mat_col = 0; mat_col < Order(local_A); mat_col++)
                        scanf("%f", 
                          (local_A->entries)+mat_row*Order(local_A)+mat_col);
                } else {
                    for(mat_col = 0; mat_col < Order(local_A); mat_col++)
                        scanf("%f", temp + mat_col);
                    MPI_Send(temp, Order(local_A), MPI_FLOAT, dest, 0,
                        grid->comm);
                }
            }
        }
        free(temp);
    } else {
        for (mat_row = 0; mat_row < Order(local_A); mat_row++) 
            MPI_Recv(&Entry(local_A, mat_row, 0), Order(local_A), 
                MPI_FLOAT, 0, 0, grid->comm, &status);
    }
                     
}  /* Read_matrix */


/*********************************************************/
void Print_matrix(
         char*            title    /* in  */,  
         LOCAL_MATRIX_T*  local_A  /* out */,
         GRID_INFO_T*     grid     /* in  */,
         int              n        /* in  */) {
    int        mat_row, mat_col;
    int        grid_row, grid_col;
    int        source;
    int        coords[2];
    float*     temp;
    MPI_Status status;

    if (grid->my_rank == 0) {
        temp = (float*) malloc(Order(local_A)*sizeof(float));
        printf("%s\n", title);
        for (mat_row = 0;  mat_row < n; mat_row++) {
            grid_row = mat_row/Order(local_A);
            coords[0] = grid_row;
            for (grid_col = 0; grid_col < grid->q; grid_col++) {
                coords[1] = grid_col;
                MPI_Cart_rank(grid->comm, coords, &source);
                if (source == 0) {
                    for(mat_col = 0; mat_col < Order(local_A); mat_col++)
                        printf("%4.1f ", Entry(local_A, mat_row, mat_col));
                } else {
                    MPI_Recv(temp, Order(local_A), MPI_FLOAT, source, 0,
                        grid->comm, &status);
                    for(mat_col = 0; mat_col < Order(local_A); mat_col++)
                        printf("%4.1f ", temp[mat_col]);
                }
            }
            printf("\n");
        }
        free(temp);
    } else {
        for (mat_row = 0; mat_row < Order(local_A); mat_row++) 
            MPI_Send(&Entry(local_A, mat_row, 0), Order(local_A), 
                MPI_FLOAT, 0, 0, grid->comm);
    }
                     
}  /* Print_matrix */


/*********************************************************/
void Set_to_zero(
         LOCAL_MATRIX_T*  local_A  /* out */) {

    int i, j;

    for (i = 0; i < Order(local_A); i++)
        for (j = 0; j < Order(local_A); j++)
            Entry(local_A,i,j) = 0.0;

}  /* Set_to_zero */


/*********************************************************/
void Build_matrix_type(
         LOCAL_MATRIX_T*  local_A  /* in */) {
    MPI_Datatype  temp_mpi_t;
    int           block_lengths[2];
    MPI_Aint      displacements[2];
    MPI_Datatype  typelist[2];
    MPI_Aint      start_address;
    MPI_Aint      address;

    MPI_Type_contiguous(Order(local_A)*Order(local_A), 
        MPI_FLOAT, &temp_mpi_t);

    block_lengths[0] = block_lengths[1] = 1;
   
    typelist[0] = MPI_INT;
    typelist[1] = temp_mpi_t;

    MPI_Address(local_A, &start_address);
    MPI_Address(&(local_A->n_bar), &address);
    displacements[0] = address - start_address;
    
    MPI_Address(local_A->entries, &address);
    displacements[1] = address - start_address;

    MPI_Type_struct(2, block_lengths, displacements,
        typelist, &local_matrix_mpi_t);
    MPI_Type_commit(&local_matrix_mpi_t); 
}  /* Build_matrix_type */


/*********************************************************/
void Local_matrix_multiply(
         LOCAL_MATRIX_T*  local_A  /* in  */,
         LOCAL_MATRIX_T*  local_B  /* in  */, 
         LOCAL_MATRIX_T*  local_C  /* out */) {
    int i, j, k;

    for (i = 0; i < Order(local_A); i++)
        for (j = 0; j < Order(local_A); j++)
            for (k = 0; k < Order(local_B); k++)
                Entry(local_C,i,j) = Entry(local_C,i,j) 
                    + Entry(local_A,i,k)*Entry(local_B,k,j);

}  /* Local_matrix_multiply */


/*********************************************************/
void Print_local_matrices(
         char*            title    /* in */,
         LOCAL_MATRIX_T*  local_A  /* in */, 
         GRID_INFO_T*     grid     /* in */) {

    int         coords[2];
    int         i, j;
    int         source;
    MPI_Status  status;

    if (grid->my_rank == 0) {
        printf("%s\n", title);
        printf("Process %d > grid_row = %d, grid_col = %d\n",
            grid->my_rank, grid->my_row, grid->my_col);
        for (i = 0; i < Order(local_A); i++) {
            for (j = 0; j < Order(local_A); j++)
                printf("%4.1f ", Entry(local_A,i,j));
            printf("\n");
        }
        for (source = 1; source < grid->p; source++) {
            MPI_Recv(temp_mat, 1, local_matrix_mpi_t, source, 0,
                grid->comm, &status);
            MPI_Cart_coords(grid->comm, source, 2, coords);
            printf("Process %d > grid_row = %d, grid_col = %d\n",
                source, coords[0], coords[1]);
            for (i = 0; i < Order(temp_mat); i++) {
                for (j = 0; j < Order(temp_mat); j++)
                    printf("%4.1f ", Entry(temp_mat,i,j));
                printf("\n");
            }
        }
        fflush(stdout);
    } else {
        MPI_Send(local_A, 1, local_matrix_mpi_t, 0, 0, grid->comm);
    }
        
}  /* Print_local_matrices */