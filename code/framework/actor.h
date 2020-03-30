#ifndef ACTOR_H_
#define ACTOR_H_

#include <mpi.h>
#include <stdbool.h>

#define MAX_LENGTH_STRING_LIMIT 32

typedef struct _Actor {
  /* Indicate the loop status of specific actor */
  bool loop_status;
  /*
   * Identify the specific actor type name in user defined way
   * However the length of name can not exceed the limit length.
   */
  char actor_type[MAX_LENGTH_STRING_LIMIT];

  /*
   * Create more actors
   * num indicating the number of new actors to be create
   * type indicating the actor type of new actors
   */
  void (*create)(struct _Actor *actor, char *type, int num);

  /*
   * Listen to an incoming message
   */
  void (*listen)(struct _Actor *actor, MPI_Status *status);

  /*
   * Specific the detail action of the actor when nothing happen
   */
  void (*execute)(struct _Actor *actor);

  /*
   * Define the previous procedure for a specific actor
   */
  void (*previous_procedure)(struct _Actor *actor);

  /*
   * Define the following procedure for a specific acto
   */
  void (*following_procedure)(struct _Actor *actor);

  /*
   * Finalize the specific actor
   */
  void (*finalize)(struct _Actor *actor);
} Actor;

/*
 * The entry to wake up the actor after the actor has been created.
 * Function will return false when fail to wake up the actor.
 */
bool wakeup_actor(Actor *actor);

#endif