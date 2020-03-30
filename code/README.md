# Biology Model Using Actor Framework
> In this project, a separate Actor Framework is provided with problem specific code which uses the framework to solve the biologist’s problem.

## How to build

The `makefile` for both framework and biologist model is provided. Project use GNU as the compiler. So build the code using

```
cd code/
make
```

## How to run

A  submission script to run the executable on Cirrus compute node(s) is provided. Using the submit command or type

```
mpirun -n 100 ./model
```

Ensure the process is enough or the program will exit abnormally.