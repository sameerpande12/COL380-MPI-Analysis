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

    if(rank==0){
       printf("Blocking \n");
       
       
        int aRows = atoi(argv[1]);
        int aCols = atoi(argv[2]);
        int bRows = aCols;
        int bCols = atoi(argv[3]);
        int cRows = aRows;
        int cCols = bCols;

        int aRowSize = aCols;
        int aColSize = aRows;
        int bRowSize = bCols;
        int bColSize = bRows;
        int cRowSize = bCols;
        int cColSize = aRows;

        float* A = (float *)malloc(sizeof(float)*aRows*aCols);
        float* B = (float *)malloc(sizeof(float)*bRows*bCols);
        float* C = (float *)malloc(sizeof(float)*aRows*bCols);

        srand48(time(0));
        for(int i =0 ;i<aRows*aCols;i++){
          A[i] = ((float)rand()) / ((float)(RAND_MAX));
        }
        for(int i =0 ;i<bRows*bCols;i++){
          B[i] = ((float)rand()) / ((float)(RAND_MAX));
        }
        double beginTime = MPI_Wtime();
        int numWorkers = numProcesses -1;
        printf("number of processes = %d, number of additional labourers = %d\n",numProcesses,numWorkers);
        int nRows=0;
        int numExtraRows = 0;
        
        nRows = aRows/numProcesses;
        numExtraRows = aRows%numProcesses;

        int startRow = 0;
        int startRowNumbers[numWorkers+1];
        int rowSentNumbers[numWorkers+1];

        for(int workerNumber = 0;workerNumber<=numWorkers;workerNumber++){
          int numRowsSent = nRows;
          if(workerNumber < numExtraRows)//need strict inequality because giving the extra worker to number zero as well
            numRowsSent++;//send additional row

          if( workerNumber > 0){
              MPI_Send(&aCols,1,MPI_INT,workerNumber,0,MPI_COMM_WORLD);
              MPI_Send(&bCols,1,MPI_INT,workerNumber,0,MPI_COMM_WORLD);
              MPI_Send(B,bRowSize*bColSize,MPI_FLOAT,workerNumber,0,MPI_COMM_WORLD);

              
              MPI_Send(&numRowsSent,1,MPI_INT,workerNumber,0,MPI_COMM_WORLD);
              MPI_Send(&startRow,1,MPI_INT,workerNumber,1,MPI_COMM_WORLD);
              startRowNumbers[workerNumber] = startRow;
              rowSentNumbers[workerNumber] = numRowsSent;
              MPI_Send( A + startRow * aRowSize, numRowsSent * aRowSize,MPI_FLOAT,workerNumber,2,MPI_COMM_WORLD);
          }
          if(workerNumber==0){
            startRowNumbers[0] = startRow;
            rowSentNumbers[0] = numRowsSent;
          }
          startRow = startRow + numRowsSent;
        }

        /*Perform the calculation for rank 0's part
        */
      //  printf("")
        for(int i = startRowNumbers[0];i<startRowNumbers[0]+rowSentNumbers[0];i++){
          for(int j = 0;j<bCols;j++){
            C[i*bCols + j ] = 0;
            for(int k = 0;k<aCols;k++){
              C[i*bCols+ j] += (A[i* aCols + k] * B[k * bCols+ j]);
            }
          }
        }

        MPI_Status status;
        for(int workerNumber =1; workerNumber <= numWorkers;workerNumber ++){
            int numRowsReceived;
            // MPI_Recv(&numRowsReceived,1,MPI_INT,workerNumber,2,MPI_COMM_WORLD,&status);
            // MPI_Recv(&startRow,1,MPI_INT,workerNumber,3,MPI_COMM_WORLD,&status);
            numRowsReceived = rowSentNumbers[workerNumber];
            startRow = startRowNumbers[workerNumber];
            MPI_Recv(&C[(startRow*cCols)],numRowsReceived * cCols,MPI_FLOAT,workerNumber,4,MPI_COMM_WORLD,&status);


        }
        double endTime = MPI_Wtime();

        printf("time = %lf seconds\n",endTime-beginTime);
        


        float * C_serial = (float *)malloc(sizeof(float * )*aRows*bCols);
        Multiply_serial(A,B,C_serial,aRows,aCols,bCols);
        // printMatrix(C,aRows,bCols);
        printf("IsEqual = %d\n\n",IsEqual(C_serial,C,aRows,bCols));
    }
    else{
        MPI_Status status;
        int numColsA,numColsB,numRows,numRowsB,startRow;
        const int source = 0;//parent
        numColsA = 0;
        numColsB = 0;
        numRows = 0;
        startRow = 0;
        // printf("Worker Number = %d\n",rank);

        MPI_Recv(&numColsA,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);
        numRowsB = numColsA;

        MPI_Recv(&numColsB,1,MPI_INT,source,0,MPI_COMM_WORLD,&status);


        float * B = (float *)malloc(sizeof(float)*numColsA * numColsB);

        MPI_Recv(B,numRowsB * numColsB,MPI_FLOAT,source,0,MPI_COMM_WORLD,&status);

        MPI_Recv(&numRows,1,MPI_FLOAT,source,0,MPI_COMM_WORLD,&status);

        MPI_Recv(&startRow,1,MPI_FLOAT,source,1,MPI_COMM_WORLD,&status);

        float * A_part = (float *)malloc(sizeof(float)* numRows * numColsA);

        MPI_Recv(A_part,numRows * numColsA,MPI_FLOAT,source,2,MPI_COMM_WORLD,&status);


        float * C = (float *)malloc(sizeof(float)*numRows*numColsB);

        for(int i = 0;i<numRows;i++){
          for(int j =0 ;j<numColsB;j++){
            C[i*numColsB + j ] = 0;
            for(int k = 0;k<numColsA;k++){
              C[i*numColsB + j] += (A_part[i* numColsA + k] * B[k * numColsB + j]);
            }
          }
        }



        // MPI_Send(&numRows,1,MPI_INT,source,2,MPI_COMM_WORLD);
        // MPI_Send(&startRow,1,MPI_INT,source,3,MPI_COMM_WORLD);
        MPI_Send(C,numRows*numColsB,MPI_INT,source,4,MPI_COMM_WORLD);

    }
    MPI_Finalize();
}
