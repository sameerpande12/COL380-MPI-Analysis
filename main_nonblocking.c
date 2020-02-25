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
        printf("Nonblocking\n");
        double beginTime = MPI_Wtime();
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
        int numWorkers = numProcesses -1;
        printf("number of workers = %d\n",numWorkers);
        int nRows=0;
        int numExtraRows = 0;
        if(numWorkers >=1){
          nRows = aRows/numWorkers;
          numExtraRows = aRows%numWorkers;
        }

        int startRow = 0;
        int startRowNumbers[numWorkers+1];
        int rowSentNumbers[numWorkers+1];

        for(int workerNumber = 1;workerNumber<=numWorkers;workerNumber++){
          MPI_Request requests[6];
          MPI_Isend(&aCols,1,MPI_INT,workerNumber,0,MPI_COMM_WORLD,&requests[0]);
          MPI_Isend(&bCols,1,MPI_INT,workerNumber,1,MPI_COMM_WORLD,&requests[1]);
          
          
          MPI_Isend(B,bRowSize*bColSize,MPI_FLOAT,workerNumber,2,MPI_COMM_WORLD,&requests[2]);

          int numRowsSent = nRows;
          if(workerNumber <= numExtraRows)
            numRowsSent++;//send additional row

          MPI_Isend(&numRowsSent,1,MPI_INT,workerNumber,3,MPI_COMM_WORLD,&requests[3]);
          MPI_Isend(&startRow,1,MPI_INT,workerNumber,4,MPI_COMM_WORLD,&requests[4]);
          startRowNumbers[workerNumber] = startRow;
          rowSentNumbers[workerNumber] = numRowsSent;


          // printf("0 %d\n",numRowsSent * aRowSize);
          MPI_Isend( A + startRow * aRowSize, numRowsSent * aRowSize,MPI_FLOAT,workerNumber,5,MPI_COMM_WORLD,&requests[5]);
          startRow = startRow + numRowsSent;
        }

        MPI_Status status;
        MPI_Request receivingRequests[numWorkers];
        for(int workerNumber =1; workerNumber <= numWorkers;workerNumber ++){
            int numRowsReceived;
           
            numRowsReceived = rowSentNumbers[workerNumber];
            startRow = startRowNumbers[workerNumber];
            MPI_Irecv(&C[(startRow*cCols)],numRowsReceived * cCols,MPI_FLOAT,workerNumber,0,MPI_COMM_WORLD,&receivingRequests[workerNumber-1]);
        }

        MPI_Waitall(numWorkers,receivingRequests,MPI_STATUSES_IGNORE);

        double endTime = MPI_Wtime();

        printf("time = %lf seconds\n\n",endTime-beginTime);
        // float * C_serial = (float *)malloc(sizeof(float * )*aRows*bCols);
        // Multiply_serial(A,B,C_serial,aRows,aCols,bCols);
        // printMatrix(C,aRows,bCols);
        // printf("IsEqual = %d\n",IsEqual(C_serial,C,aRows,bCols));
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
        MPI_Request requests[7];
        MPI_Irecv(&numColsA,1,MPI_INT,source,0,MPI_COMM_WORLD,&requests[0]);
        MPI_Irecv(&numColsB,1,MPI_INT,source,1,MPI_COMM_WORLD,&requests[1]);
        MPI_Waitall(2,requests,MPI_STATUSES_IGNORE);
        numRowsB = numColsA;        

        // printf("%d %d ",numRowsB,numColsA);
        float * B = (float *)malloc(sizeof(float)*numColsA * numColsB);

        MPI_Irecv(B,numRowsB * numColsB,MPI_FLOAT,source,2,MPI_COMM_WORLD,&requests[2]);

        MPI_Irecv(&numRows,1,MPI_FLOAT,source,3,MPI_COMM_WORLD,&requests[3]);

        MPI_Irecv(&startRow,1,MPI_FLOAT,source,4,MPI_COMM_WORLD,&requests[4]);


        MPI_Waitall(3,&requests[2],MPI_STATUSES_IGNORE);//necessary since we need numRows to initialize the A_part matrix


        float * A_part = (float *)malloc(sizeof(float)* numRows * numColsA);

        MPI_Irecv(A_part,numRows * numColsA,MPI_FLOAT,source,5,MPI_COMM_WORLD,&requests[5]);

        MPI_Wait(&requests[5],MPI_STATUS_IGNORE);


        float * C = (float *)malloc(sizeof(float)*numRows*numColsB);

        for(int i = 0;i<numRows;i++){
          for(int j =0 ;j<numColsB;j++){
            C[i*numColsB + j ] = 0;
            for(int k = 0;k<numColsA;k++){
              C[i*numColsB + j] += (A_part[i* numColsA + k] * B[k * numColsB + j]);
            }
          }
        }

        MPI_Isend(C,numRows*numColsB,MPI_INT,source,0,MPI_COMM_WORLD,&requests[6]);

    }
    MPI_Finalize();
}
