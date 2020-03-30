#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../framework/actor.h"
#include "../framework/pool.h"
#include "biology_factory.h"
#include "biology_config.h"

void timer_listen(Actor *actor, MPI_Status *status);
void timer_execute(Actor *actor);
void timer_previous_procedure(Actor *actor);
void timer_finalize(Actor *actor);

extern int proc;                              
int current_month = 0;                       
int timer_cell_proc[NUM_OF_CELL];  
int health_count = 0, infected_count = 0, total_count = 0;
time_t start, curr;

void init_timer_actor(Actor *actor) {
  strncpy(actor->actor_type, "TIMER", MAX_LENGTH_STRING_LIMIT);
  actor->loop_status = true;
  actor->listen = &timer_listen;
  actor->execute = &timer_execute;
  actor->create = NULL;
  actor->previous_procedure = &timer_previous_procedure;
  actor->following_procedure = NULL;
  actor->finalize = &timer_finalize;
  start = time(&start);
  curr = time(&curr);
}

void timer_listen(Actor *actor, MPI_Status *status) {
  int count;
  int source = status->MPI_SOURCE;
  int tag = status->MPI_TAG;
  switch (tag) {
    case SQUIRREL_FINALIZE_TAG:
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      total_count -= 1;
      infected_count -= 1;
      break;
    case SQUIRREL_INFECT_TAG:
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      health_count -= 1;
      infected_count += 1;
      break;
    case SQUIRREL_BORN_TAG:
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      total_count += 1;
      health_count += 1;
      break;
    default:
      break;
  }
}

void timer_execute(Actor *actor) {
  time(&curr);
  if (difftime(curr, start) >= STEP_OF_MONTH) {
    time(&start);
    int buf[2];
    fprintf(stdout, "Month: %3d, Health_Squirrel: %d, Infected_Squirrel: %d, Total_Squirrel: %d\n",
            current_month, health_count, infected_count, total_count);
    fprintf(stdout, "%-25s%-25s%-25s\n", "cell", "population_influx",
            "infection_level");
    for (int i = 0; i < NUM_OF_CELL; ++i) {
      MPI_Ssend(NULL, 0, MPI_INT, timer_cell_proc[i], CELL_QUERY_TAG,
                MPI_COMM_WORLD);
      MPI_Recv(buf, 2, MPI_INT, timer_cell_proc[i], CELL_QUERY_TAG,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      fprintf(stdout, "%-25d%-25d%-25d\n", i + 1, buf[1], buf[0]);
    }
    fprintf(stdout, "\n");
    current_month += 1;
    if (current_month >= LIMIT_OF_MONTH || total_count >= LIMIT_SQUIRREL) {
      actor->finalize(actor);
    }
  }
}

void timer_previous_procedure(Actor *actor) {
  MPI_Recv(timer_cell_proc, NUM_OF_CELL, MPI_INT, 0, 100, MPI_COMM_WORLD,
           MPI_STATUS_IGNORE);
}

void timer_finalize(Actor *actor) {
  shutdownPool();
  actor->loop_status = false;
  MPI_Abort(MPI_COMM_WORLD, SIMULATION_ERROR);
}