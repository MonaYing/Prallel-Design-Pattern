#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "actor.h"
#include "pool.h"

bool wakeup_actor(Actor *actor) {
  if (!actor) {
    fprintf(stderr, "Error: cannot wake up an actor which is not created.\n");
    return false;
  }
  if (actor->previous_procedure) {
    actor->previous_procedure(actor);
  }
  MPI_Status status;
  int flag = 0;
  while (actor->loop_status) {
    flag = 0;
    while (!flag && actor->loop_status) {
      MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
      if (actor->execute) {
        actor->execute(actor);
      }
    }
    if (actor->listen) {
      actor->listen(actor, &status);
    }
  }
  if (actor->following_procedure) {
    actor->following_procedure(actor);
  }
  return true;
}