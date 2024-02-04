# Flamenco
Project for Distributed Processing course at university. Entirety written i C, using MPI for distributed computing.

## Specification
W małej hiszpańskiej wioseczce mieszkają gitarzyści i tancerki flamenco. Od czasu do czasu dobierają się w pary i dają występy.

Danych jest G gitarzytów i F tancerek. Najpierw muszą się dobrać w pary. Następnie ubiegają się o jedną z S rozróżnialnych sal. Później dają występ.

## Compiling
`make debug` for verbose version (debugging info).

`make` for normal version (only info required by project specifications)

## Running
`make run` - compile normal version and run `mpirun -oversubscribe -np 8 ./main`

`mpirun -np [number_of_processes] ./main` - run [number_of_processes] instances.
