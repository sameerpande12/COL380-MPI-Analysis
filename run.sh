mpicc main_collective.c -o main_collective.exe
mpirun -np $2 ./main_collective.exe $1 32 $1
