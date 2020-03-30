#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../framework/actor.h"
#include "../framework/pool.h"
#include "biology_config.h"
#include "biology_factory.h"
#include "squirrel-functions.h"

/* Framework users define the functions, passing the function pointer into
 * struct Actor */
void squirrel_listen(Actor *actor, MPI_Status *status);
void squirrel_execute(Actor *actor);
void squirrel_create(Actor *actor, char *type, int count);
void squirrel_previous_procedure(Actor *actor);
void squirrel_finalize(Actor *actor);

void step_update(int new_infection_level, int new_population_influx);
float average_population();
float average_infection();

/* Extern variable indicating the process id */
extern int proc;
/* Random seed */
long seed;
/* Squirrel’s current position in the environment */
float position[2] = {0.0, 0.0};
int squirrel_cell_proc[NUM_OF_CELL];
int clock_proc;
/* Squirrel's health state */
int health_state = 1;
/* Total step counter */
int total_step_counter = 0;
/* Infection step counter */
int infection_step_counter = 0;
/* Contain the infection level history */
int infection_level_history[NUM_OF_HISTORY_STEP];
/* Contain the population level history */
int population_influx_history[NUM_OF_HISTORY_STEP];

void init_squirrel_actor(Actor *actor) {
  strncpy(actor->actor_type, "SQUIRREL", MAX_LENGTH_STRING_LIMIT);
  actor->loop_status = true;
  actor->listen = &squirrel_listen;
  actor->execute = &squirrel_execute;
  actor->create = &squirrel_create;
  actor->previous_procedure = &squirrel_previous_procedure;
  actor->following_procedure = NULL;
  actor->finalize = &squirrel_finalize;
  seed = -1 - proc;
  initialiseRNG(&seed);
  memset(infection_level_history, 0, NUM_OF_HISTORY_STEP * sizeof(int));
  memset(population_influx_history, 0, NUM_OF_HISTORY_STEP * sizeof(int));
}

void squirrel_listen(Actor *actor, MPI_Status *status) {
  int count;
  int source = status->MPI_SOURCE;
  int tag = status->MPI_TAG;

  switch (tag) {
    case SQUIRREL_INFECT_TAG: {
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      health_state = 0;
      total_step_counter = 50;
      break;
    }
    case SQUIRREL_FINALIZE_TAG:
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      actor->finalize(actor);
      break;
    default:
      break;
  }
}

void squirrel_execute(Actor *actor) {
  float new_x, new_y;
  squirrelStep(position[0], position[1], &new_x, &new_y, &seed);
  int cell = getCellFromPosition(new_x, new_y);
  position[0] = new_x;
  position[1] = new_y;
  total_step_counter += 1;
  if (!health_state) {
    infection_step_counter++;
  }

  int infection_level, population_influx;
  int buf[2];
  MPI_Send(&health_state, 1, MPI_INT, squirrel_cell_proc[cell], CELL_HOP_TAG,
           MPI_COMM_WORLD);
  MPI_Recv(buf, 2, MPI_INT, squirrel_cell_proc[cell], CELL_HOP_TAG,
           MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  infection_level = buf[0];
  population_influx = buf[1];

  step_update(infection_level, population_influx);
  if ((total_step_counter % NUM_OF_HISTORY_STEP) == 0 &&
      willGiveBirth(average_population(), &seed)) {
    actor->create(actor, "SQUIRREL", 1);
    MPI_Send(NULL, 0, MPI_INT, clock_proc, SQUIRREL_BORN_TAG, MPI_COMM_WORLD);
  }
  if (total_step_counter > NUM_OF_HISTORY_STEP && health_state &&
      willCatchDisease(average_infection(), &seed)) {
    health_state = 0;
    MPI_Send(NULL, 0, MPI_INT, clock_proc, SQUIRREL_INFECT_TAG, MPI_COMM_WORLD);
  }
  if (!health_state && infection_step_counter >= DEAD_STEP && willDie(&seed)) {
    actor->finalize(actor);
    MPI_Send(NULL, 0, MPI_INT, clock_proc, SQUIRREL_FINALIZE_TAG,
             MPI_COMM_WORLD);
  }
  struct timespec tim, tim2;
  tim.tv_sec = 0;
  tim.tv_nsec = SQUIRREL_SLEEP;
  nanosleep(&tim, &tim2);
}

void squirrel_create(Actor *actor, char *type, int count) {
  int squirrel_proc;
  for (int i = 0; i < count; ++i) {
    squirrel_proc = startWorkerProcess();
    float buf[2 + NUM_OF_CELL + 1];
    buf[0] = position[0];
    buf[1] = position[1];
    for (int i = 2; i < 2 + NUM_OF_CELL; ++i) {
      buf[i] = (float)squirrel_cell_proc[i - 2];
    }
    buf[2 + NUM_OF_CELL] = (float)clock_proc;
    MPI_Send("SQUIRREL", 9, MPI_CHAR, squirrel_proc, 100, MPI_COMM_WORLD);
    MPI_Send(buf, 2 + NUM_OF_CELL + 1, MPI_FLOAT, squirrel_proc,
             SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD);
  }
}

void squirrel_previous_procedure(Actor *actor) {
  float buf[2 + NUM_OF_CELL + 1];
  MPI_Recv(buf, 2 + NUM_OF_CELL + 1, MPI_FLOAT, MPI_ANY_SOURCE,
           SQUIRREL_PREPROCESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  position[0] = buf[0];
  position[1] = buf[1];
  for (int i = 2; i < 2 + NUM_OF_CELL; ++i) {
    squirrel_cell_proc[i - 2] = (int)buf[i];
  }
  clock_proc = (int)buf[2 + NUM_OF_CELL];
}

void squirrel_finalize(Actor *actor) { actor->loop_status = false; }

void step_update(int new_infection_level, int new_population_influx) {
  infection_level_history[total_step_counter % NUM_OF_HISTORY_STEP] =
      new_infection_level;
  population_influx_history[total_step_counter % NUM_OF_HISTORY_STEP] =
      new_population_influx;
}

float average_population() {
  float res = 0;
  for (int i = 0; i < NUM_OF_HISTORY_STEP; ++i) {
    res += population_influx_history[i];
  }
  res /= NUM_OF_HISTORY_STEP;
  return res;
}

float average_infection() {
  float res = 0;
  for (int i = 0; i < NUM_OF_HISTORY_STEP; ++i) {
    res += infection_level_history[i];
  }
  res /= NUM_OF_HISTORY_STEP;
  return res;
}