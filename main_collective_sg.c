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
    int aRowsActual;
    int aRows,aCols,bCols;
    double begin,end;

    float* A,*B,*C;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesses);
    int numWorkers = numProcesses ;
    if(numProcesses<=1){
      printf("Number of Processes should be greater than 1\n");
      MPI_Finalize();
      return 0;
    }
    
    if(rank==0){

        printf("Collective SG \n");
        
       
        aRowsActual = atoi(argv[1]);

        if( aRowsActual % numWorkers == 0 ){
            aRows = aRowsActual;
        }
        else{
            aRows = (aRowsActual/numWorkers + 1)*numWorkers; 
        }

        aCols = atoi(argv[2]);
        
        
        int bRows = aCols;
        bCols = atoi(argv[3]);
        
        
        int cRows = aRows;
        int cCols = bCols;

        

        int aRowSize = aCols;
        int aColSize = aRows;
        int bRowSize = bCols;
        int bColSize = bRows;
        int cRowSize = bCols;
        int cColSize = aRows;

        A = (float *)malloc(sizeof(float)*aRows*aCols);
        B = (float *)malloc(sizeof(float)*bRows*bCols);
        C = (float *)malloc(sizeof(float)*aRows*bCols);
        
        srand48(time(0));
        for(int i =0 ;i<aRows*aCols;i++){
          A[i] = ((float)rand()) / ((float)(RAND_MAX));
          if(i>=aRowsActual*aCols){
              A[i]=0;
          }
        }
        for(int i =0 ;i<bRows*bCols;i++){
          B[i] = ((float)rand()) / ((float)(RAND_MAX));
        }
        

        // MPI_Bcast(&aRows,1,MPI_INT,0,MPI_COMM_WORLD);
        // MPI_Bcast(&aCols,1,MPI_INT,0,MPI_COMM_WORLD);
        // MPI_Bcast(&bCols,1,MPI_INT,0,MPI_COMM_WORLD);
        // MPI_Bcast(B,aCols*bCols,MPI_FLOAT,0,MPI_COMM_WORLD);

    }
    MPI_Barrier(MPI_COMM_WORLD);
    begin = MPI_Wtime();

    MPI_Bcast(&aRows,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&aCols,1,MPI_INT,0,MPI_COMM_WORLD);
    MPI_Bcast(&bCols,1,MPI_INT,0,MPI_COMM_WORLD);
    
    MPI_Barrier(MPI_COMM_WORLD);//ensuring that all the values have been updated
    
    float * A_part = (float*)malloc(aRows/numWorkers * aCols * sizeof(float));
    float * C_part = (float*)malloc(aRows/numWorkers * bCols * sizeof(float));
    B = (float *)malloc(aCols * bCols *sizeof(float)); 
    // if(rank != 0){
    C = (float *)malloc(aRows * bCols * sizeof(float));
    

    MPI_Bcast(B,aCols * bCols,MPI_FLOAT,0,MPI_COMM_WORLD);
    MPI_Scatter(A, aRows/numWorkers * aCols, MPI_FLOAT, A_part, aRows/numWorkers * aCols, MPI_FLOAT,0,MPI_COMM_WORLD);

    int rowsReceived = aRows/numWorkers;
    
    for(int i = 0;i<rowsReceived;i++){
        for(int j = 0;j<bCols;j++){
            C_part[ i * bCols + j] = 0;

            for(int k = 0;k<aCols;k++){
                C_part[i*bCols + j] += ( A_part[i*aCols + k] *B[k*bCols+j]);
            }
        }
    }

    MPI_Gather(C_part,aRows/numWorkers * bCols, MPI_FLOAT,C,aRows/numWorkers * bCols,MPI_FLOAT,0,MPI_COMM_WORLD);
    // printf("rank = %d,  rowsReceived = %d\n",rank,rowsReceived);
    
    MPI_Barrier(MPI_COMM_WORLD);


    



    
    if(rank == 0){
        end = MPI_Wtime();
        float * C_actual;
        // C_actual = (float *)malloc( aRowsActual * bCols * sizeof(float));
        
        // for(int i = 0;i<aRowsActual * bCols;i++){
        //     C_actual[i]=C[i];
        // }
        if(aRowsActual !=aRows){
            C_actual = (float *)malloc( aRowsActual * bCols * sizeof(float));
            for(int i = 0;i<aRowsActual*bCols;i++)
                C_actual[i] = C[i];
        }
        else{
            C_actual = C;
        }

        float * C_serial = (float *)malloc(sizeof(float)*aRows*bCols);
        Multiply_serial(A,B,C_serial,aRows,aCols,bCols);
        
        printf("time = %f seconds\n",end - begin);
        printf("IsEqual = %d \n\n",IsEqual(C_serial,C,aRows,bCols));

    }


    MPI_Finalize();
}
