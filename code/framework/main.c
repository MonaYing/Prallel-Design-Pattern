#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actor.h"
#include "factory.h"
#include "pool.h"

#define MAIN_PROCESS 0
#define buf_size 256 * 1024 
int nprocs, proc;
Actor actor; 
char mpi_buf[buf_size];

int main(int argc, char **argv) {
  /* Initial MPI environment */
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &proc);
  MPI_Buffer_attach(mpi_buf, buf_size);
  int statusCode = processPoolInit();
  if (statusCode == 1) {
    /* Get the type of worker actor from main actor*/
    char type[MAX_LENGTH_STRING_LIMIT];
    memset(type, '\0', MAX_LENGTH_STRING_LIMIT);
    MPI_Recv(type, MAX_LENGTH_STRING_LIMIT, MPI_CHAR, MAIN_PROCESS, 100,
             MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    // printf("%s\n", type);
    init_worker_actor(type, &actor);
    wakeup_actor(&actor);
  } else if (statusCode == 2) {
    init_main_actor(&actor);
    wakeup_actor(&actor);
  }
  int detach_buf_size = buf_size;
  MPI_Buffer_detach(mpi_buf, &detach_buf_size);
  processPoolFinalise();
  MPI_Finalize();
  return 0;
}