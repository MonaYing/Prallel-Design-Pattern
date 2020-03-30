#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../framework/actor.h"
#include "../framework/pool.h"
#include "biology_factory.h"
#include "biology_config.h"

void cell_listen(Actor *actor, MPI_Status *status);
void cell_finalize(Actor *actor);
void monthly_update();
int get_infection_level();
int get_population_influx();

extern int proc;
int month;                      // current month
int current_infection_level;    // infection_level this month
int current_population_influx;  // population_influx this month
int infection_level[2];         // infection level history
int population_influx[3];       // population influx history

void init_cell_actor(Actor *actor) {
  strncpy(actor->actor_type, "CELL", MAX_LENGTH_STRING_LIMIT);
  actor->loop_status = true;
  actor->listen = &cell_listen;
  actor->execute = NULL;
  actor->create = NULL;
  actor->previous_procedure = NULL;
  actor->following_procedure = NULL;
  actor->finalize = &cell_finalize;
  int month = 0;
  int current_infection_level = 0;
  int current_population_influx = 0;
  memset(infection_level, 0, sizeof(int) * 2);
  memset(population_influx, 0, sizeof(int) * 3);
}

void cell_listen(Actor *actor, MPI_Status *status) {
  int count;
  int source = status->MPI_SOURCE;
  int tag = status->MPI_TAG;
  switch (tag) {
    case CELL_HOP_TAG: {
      int health;
      int buf[2];
      buf[0] = get_infection_level();
      buf[1] = get_population_influx();
      MPI_Recv(&health, 1, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Send(buf, 2, MPI_INT, source, tag, MPI_COMM_WORLD);
      ++current_population_influx;
      if (!health) {
        ++current_infection_level;
      }
      break;
    }
    case CELL_QUERY_TAG: {
      int buf[2]; 
      buf[0] = get_infection_level();
      buf[1] = get_population_influx();

      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      MPI_Send(buf, 2, MPI_INT, source, tag, MPI_COMM_WORLD);
      monthly_update();
      current_infection_level = 0;
      current_population_influx = 0;
      ++month;
      break;
    }
    case CELL_FINALIZE_TAG:
      MPI_Recv(NULL, 0, MPI_INT, source, tag, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);
      actor->finalize(actor);
      break;
    default:
      break;
  }
}

void cell_finalize(Actor *actor) { actor->loop_status = false; }

void monthly_update() {
  infection_level[month % 2] = current_infection_level;
  population_influx[month % 3] = current_population_influx;
}

int get_infection_level() { return infection_level[0] + infection_level[1]; }

int get_population_influx() {
  return population_influx[0] + population_influx[1] + population_influx[2];
}