#include <mpi.h>
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
            if(A[i*n + j] != B[i*n + j])
                return 0;
        }
    }
    return 1;
}

int main(int argc, char**argv){

    int rank,numProcesses;
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcesses);

    if(rank==0){

        int aRows = atoi(argv[1]);
        int aCols = atoi(argv[2]);
        int bRows = aCols;
        int bCols = atoi(argv[3]);

        int aRowSize = aCols;
        int aColSize = aRows;
        int bRowSize = bCols;
        int bColSize = bRows;

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

        int nRows=0;
        int numExtraRows = 0;
        if(numWorkers >=1){
          nRows = aRows/numWorkers;
          numExtraRows = aRows%numWorkers;
        }

        int startRow = 0;
        MPI_Bcast(&aCols,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(&bCols,1,MPI_INT,0,MPI_COMM_WORLD);
        MPI_Bcast(B,bRowSize*bColSize,MPI_FLOAT,0,MPI_COMM_WORLD);

        // printf("Parent's B -->\n");
        // for(int i = 0;i<bRows;i++){
        //   for(int j =0;j<bCols;j++){
        //     printf("%f ",B[i * bRowSize + j]);
        //   }
        //   printf("\n");
        // }
        // for(int workerNumber = 1;workerNumber<=numWorkers;workerNumber++){
        //
        //   int numRowsSent = nRows;
        //   if(workerNumber <= numExtraRows)
        //     numRowsSent++;//send additional row
        //
        //
        //   MPI_Send(&numRowsSent,1,MPI_INT,workerNumber,0,MPI_COMM_WORLD);
        //   MPI_Send(&A[ startRow * aRowSize], numRowsSent * aRowSize,MPI_FLOAT,workerNumber,1,MPI_COMM_WORLD);
        //
        //
        //   startRow = startRow + numRowsSent;
        // }
    }
    else{
        int numColsA,numColsB,source,numRows,numRowsB;
        source = 0;//parent
        numColsA = 0;
        numColsB = 0;
        numRows = 0;
        printf("Worker Number = %d\n",rank);

        MPI_Bcast(&numColsA,1,MPI_INT,source,MPI_COMM_WORLD);
        numRowsB = numColsA;

        printf("numRowsB = %d\n",numColsA);

        MPI_Bcast(&numColsB,1,MPI_INT,source,MPI_COMM_WORLD);
        printf("numColsB = %d\n",numColsB);
        // float * C = (float *)malloc(sizeof(float)*numRows*numColsB);
        float * B = (float *)malloc(sizeof(float)*numColsA * numColsB);


        MPI_Bcast(B,numRowsB * numColsB,MPI_FLOAT,0,MPI_COMM_WORLD);
        //
        // for(int i =0;i<numRowsB;i++){
        //   for(int j =0;j<numColsB;j++){
        //     printf("%f ",B[i*numColsB + j]);
        //   }
        //   printf("\n");
        // }


    }
    MPI_Finalize();


  // int N = atoi(argv[1]);
  // printf("%d\n",N);
  // MPI_Init(NULL,NULL);
  // int world_size;
  // MPI_Comm_size(MPI_COMM_WORLD,&world_size);
  //
  // int world_rank;
  // MPI_Comm_rank(MPI_COMM_WORLD,&world_rank);
  //
  // char processor_name[MPI_MAX_PROCESSOR_NAME];
  // int name_len;
  // MPI_Get_processor_name(processor_name,&name_len);
  // printf("Hello world from processor %s, rank %d out of %d processors\n",processor_name,world_rank,world_size);
  // MPI_Finalize();
}
