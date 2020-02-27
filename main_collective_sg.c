#include </usr/include/mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
// #include <random>
#include <time.h>
#include <unistd.h>
void Multiply_serial(float* A,float *B, float *C, int m,int n, int p ){
//A is mXn,  B is n X p
  int i,j,k;
  for(i=0;i<m;i++){
    for(j=0;j<p;j++){
      C[i*p+j]=0;
      for(k = 0;k<n;k++){
        C[i*p + j] += A[i*n + k] * B[k * p + j];
      }
    }
  }
}

int IsEqual(float *A, float *B, int m, int n){
    for(int i = 0;i<m;i++){
        for(int j = 0;j<n;j++){
            if(A[i*n + j] != B[i*n + j]){
              printf("difference %d %d %f %f\n",i,j,A[i*n + j],B[i*n + j]);
              return 0;
            }

        }
    }
    return 1;
}

void printMatrix(float * matrix,int nrows,int ncols){
  for(int i = 0;i < nrows ; i++){
    for(int j = 0; j<ncols ; j++){
      printf("%f ",matrix[i*ncols + j]);
    }
    printf("\n");
  }
  printf("\n");
}

int main(int argc, char**argv){

    int rank,numProcesses;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesses);
    
    if(numProcesses<=1){
      printf("Number of Processes should be greater than 1\n");
      MPI_Finalize();
      return 0;
    }

    
    MPI_Finalize();
}
