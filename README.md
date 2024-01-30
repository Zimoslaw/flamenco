# flamenco
Project for Distributed Processing course at university. Entirety written i C, using MPI for distributed computing.

## Compiling
`make debug` for verbose version (debugging info).

`make` for normal version (only info required by project specifications)

## Running
`make run` - compile normal version and run `mpirun -oversubscribe -np 8 ./main`

`mpirun -np [number_of_processes] ./main` - run [number_of_processes] instances.
