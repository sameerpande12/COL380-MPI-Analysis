mpicc main_blocking.c -o main_blocking.exe
mpirun -np $2 ./main_blocking.exe $1 32 $1
