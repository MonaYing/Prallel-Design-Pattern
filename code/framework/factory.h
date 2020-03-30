#ifndef FACTORY_H_
#define FACTORY_H_
#include "actor.h"

/*
 * Functions below will initial the actor for main and worker process
 * User must implement these two functions in order to use the framework 
 * we supply them.
 */
void init_worker_actor(char *type, Actor *actor);
void init_main_actor(Actor *actor);
#endif