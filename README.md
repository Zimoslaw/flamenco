# Flamenco
Project for Distributed Processing course at university. Entirety written i C, using MPI for distributed computing.

## Specification
W małej hiszpańskiej wioseczce mieszkają gitarzyści i tancerki flamenco. Od czasu do czasu dobierają się w pary i dają występy.

Danych jest G gitarzytów i T tancerek. Najpierw muszą się dobrać w pary. Następnie ubiegają się o jedną z S rozróżnialnych sal. Później dają występ.

## Specifying numbers T, G and S
Number **T** is set in file `watek_glowny.c` in definition `#define NUM_OF_TANCERKI`

Number **G** is equal to SIZE-T. Where SIZE is the size of MPI domain (number of processes)

Number **S** is set in file `watek_glowny.c` in definition `#define ROOMS`

## Run time
Process runs in infinite loop. Process ends when variable `changes` (`watek_glowny.c`) reaches `#define MAX_STATE_CHANGES` (`main.h`)

`changes` is incremented with each state change caused by received packet

## Compiling
`make debug` for verbose version (debugging info).

`make` for normal version (only info required by project specifications)

## Running
`make run` - compile normal version and run `mpirun -np 8 --oversubscribe ./main`

`mpirun -np [number_of_processes] ./main` - run [number_of_processes] instances.
