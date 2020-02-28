numProcesses=`expr $2`

mpicc main_nonblocking.c -O3 -o main_nonblocking.exe
mpirun -np $numProcesses ./main_nonblocking.exe $1 32 $1

mpicc main_blocking.c -O3 -o main_blocking.exe
mpirun -np $numProcesses ./main_blocking.exe $1 32 $1

mpicc main_collective.c -O3 -o main_collective.exe
mpirun -np $numProcesses ./main_collective.exe $1 32 $1


mpicc main_collective_sg.c -O3 -o main_collective_sg.exe
mpirun -np $numProcesses ./main_collective_sg.exe $1 32 $1
