#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../framework/actor.h"
#include "../framework/factory.h"
#include "../framework/pool.h"
#include "biology_config.h"
#include "squirrel-functions.h"

void main_previous_procedure(Actor *actor);
void main_following_procedure(Actor *actor);

extern int proc;
long seed;                           
int cell_to_proc[NUM_OF_CELL]; 

void init_main_actor(Actor *actor) {
  strncpy(actor->actor_type, "MAIN", MAX_LENGTH_STRING_LIMIT);
  actor->loop_status = false;
  actor->listen= NULL;
  actor->execute = NULL;
  actor->create = NULL;
  actor->previous_procedure = &main_previous_procedure;
  actor->following_procedure = &main_following_procedure;
  actor->finalize = NULL;
  memset(cell_to_proc, -1, sizeof(int) * NUM_OF_CELL);
  seed = -1 - proc;
  initialiseRNG(&seed);
}

void main_previous_procedure(Actor *actor) {
  MPI_Request cell_requests[NUM_OF_CELL];
  int cell_proc;
  /* Create cell worker process */
  for (int i = 0; i < NUM_OF_CELL; ++i) {
    cell_proc = startWorkerProcess();
    MPI_Issend("CELL", 5, MPI_CHAR, cell_proc, 100, MPI_COMM_WORLD,
               &cell_requests[i]);
    cell_to_proc[i] = cell_proc;
  }
  MPI_Waitall(NUM_OF_CELL, cell_requests, MPI_STATUS_IGNORE);
  /* Create timer worker process */
  int timer_proc = startWorkerProcess();
  MPI_Ssend("TIMER", 6, MPI_CHAR, timer_proc, 100, MPI_COMM_WORLD);
  MPI_Ssend(cell_to_proc, NUM_OF_CELL, MPI_INT, timer_proc, 100, MPI_COMM_WORLD);
  float buf[2 + NUM_OF_CELL + 1];
  buf[0] = 0.0, buf[1] = 0.0;
  for (int i = 2; i < 2 + NUM_OF_CELL; ++i) {
    buf[i] = (float)cell_to_proc[i - 2];
  }
  buf[2 + NUM_OF_CELL] = (float)timer_proc;
  /* Create squirrel work process */
  int squirrel_proc;
  for (int i = 0; i < NUM_OF_SQUIRREL; ++i) {
    squirrel_proc = startWorkerProcess();
    MPI_Ssend("SQUIRREL", 9, MPI_CHAR, squirrel_proc, 100, MPI_COMM_WORLD);
    MPI_Ssend(buf, 2 + NUM_OF_CELL + 1, MPI_FLOAT, squirrel_proc,
              SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);
    MPI_Ssend(NULL, 0, MPI_INT, timer_proc, SQUIRREL_BORN_TAG, MPI_COMM_WORLD);
    /* Create the initial infected squirrels */
    if (i < INIT_SQUIRREL_INFECT) {
      MPI_Ssend(NULL, 0, MPI_INT, squirrel_proc, SQUIRREL_INFECT_TAG,
                MPI_COMM_WORLD);
      MPI_Ssend(NULL, 0, MPI_INT, timer_proc, SQUIRREL_INFECT_TAG,
                MPI_COMM_WORLD);
    }
  }
}

void main_following_procedure(Actor *actor) {
  int main_actor_status = masterPoll();
  while (main_actor_status) {
    main_actor_status = masterPoll();
  }
}