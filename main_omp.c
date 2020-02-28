#include </usr/include/mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
// #include <random>
#include <time.h>
#include <unistd.h>
#include <omp.h>
void Multiply_serial(float* A,float *B, float *C, int m,int n, int p ){
//A is mXn,  B is n X p
  int i,j,k;
  for(i=0;i<m;i++){
    for(j=0;j<p;j++){
      
      float temp=0;
      for(k = 0;k<n;k++){
        temp += A[i*n + k] * B[k * p + j];
      }
      C[i*p+j] = temp;
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
        int threadCount = atoi(argv[4]);
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

        float* C_serial = (float *)malloc(sizeof(float)*aRows*bCols);

        srand48(time(0));
        for(int i =0 ;i<aRows*aCols;i++){
          A[i] = ((float)rand()) / ((float)(RAND_MAX));
        }
        for(int i =0 ;i<bRows*bCols;i++){
          B[i] = ((float)rand()) / ((float)(RAND_MAX));
        }
        // printMatrix(A,aRows,aCols);
        double beginTime = omp_get_wtime();
        // Multiply_serial(A,B,C,aRows,aCols,bCols);
        
            #pragma omp parallel for num_threads(threadCount)
            for(int i=0;i<aRows;i++){
                for(int j=0;j<bCols;j++){
            
                    float temp=0;
                    for(int k = 0;k<aCols;k++){
                        temp += A[i*aCols + k] * B[k * bCols + j];
                    }
                    C[i*bCols+j] = temp;
            }
           }
            
        
        double endTime = omp_get_wtime();

        printf("omp time = %lf seconds\n",endTime-beginTime);
        // beginTime = omp_get_wtime();
        Multiply_serial(A,B,C_serial,aRows,aCols,bCols);
        // endTime = omp_get_wtime();

        // printf("serial time = %lf seconds, N = %d\n",endTime-beginTime,aRows);

        printf("%d isEqual ",IsEqual(C_serial,C,aRows,bCols));

}
