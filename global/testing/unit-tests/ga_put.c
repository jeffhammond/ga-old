/*
 * Test Program for GA
 * This is to test GA_Put (is a collective operation)
 * GA_Create -- used to create a global array using handles like 'g_A' 
 */

#include<stdio.h>
#include<stdlib.h>

#include"mpi.h"
#include"ga.h"
#include"macdecls.h"
#include"ga_unit.h"

#define DIM 2
#define SIZE 5

main(int argc, char **argv)
{
  int rank, nprocs, i, j;
  int g_A, **local_A=NULL, **local_B=NULL; 
  int dims[DIM]={SIZE,SIZE}, dims2[DIM], lo[DIM]={SIZE-SIZE,SIZE-SIZE}, hi[DIM]={SIZE-1,SIZE-1}, ld=5, value=5;

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

  MA_init(C_INT, 1000, 1000);

  GA_Initialize();


  local_A=(int**)malloc(SIZE*sizeof(int*));
  for(i=0; i<SIZE; i++)
    {
      local_A[i]=(int*)malloc(SIZE*sizeof(int));
      for(j=0; j<SIZE; j++) local_A[i][j]=rand()%10;
    }

  local_B=(int**)malloc(SIZE*sizeof(int*));
  for(i=0; i<SIZE; i++)
    {
      local_B[i]=(int*)malloc(SIZE*sizeof(int));
      for(j=0; j<SIZE; j++) local_B[i][j]=rand()%10;
    }

  g_A = NGA_Create(C_INT, DIM, dims, "array_A", NULL);
  //GA_Fill(g_A, &value);
  //  GA_Zero(g_A);
  GA_Print(g_A);

  NGA_Put(g_A, lo, hi, local_A, &ld);
  
  NGA_Get(g_A, lo, hi, local_B, &ld);

  GA_Sync();
  GA_Print(g_A);

  if(rank==0)
    {
      printf(" local_B \n");
      for(i=0; i<SIZE; i++)
	{
	  for(j=0; j<SIZE; j++)
	    printf("%d ", local_B[i][j]);
	  printf("\n");
	}

      printf("\n");
      printf(" local_A \n");
      for(i=0; i<SIZE; i++)
	{
	  for(j=0; j<SIZE; j++)
	    printf("%d ", local_A[i][j]);
	  printf("\n");
	}
      
      printf("\n");
      for(i=0; i<SIZE; i++)
	{
	  for(j=0; j<SIZE; j++)
	    {
	      if(local_B[i][j]!=local_A[i][j])
		printf("ERROR : in passing values \n");
	      /* there is erroe in the above piece of code 
	       * have to find method to solve it
	       */
	    }
	}
    }
  //  GA_Print(g_A);
  GA_Sync();
  //GA_Destroy(g_A);
  if(rank == 0)
    GA_PRINT_MSG();

  GA_Terminate();
  MPI_Finalize();
}
