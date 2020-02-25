mpicc main_nonblocking.c -o main_nonblocking.exe
mpirun -np $2 ./main_nonblocking.exe $1 32 $1
