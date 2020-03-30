#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../framework/factory.h"
#include "biology_factory.h"

void init_worker_actor(char *type, Actor *actor) {
  if (actor) {
    if (strcmp(type, "TIMER") == 0) {
      init_timer_actor(actor);
    } else if (strcmp(type, "CELL") == 0) {
      init_cell_actor(actor);
    } else if (strcmp(type, "SQUIRREL") == 0) {
      init_squirrel_actor(actor);
    }
  }
}