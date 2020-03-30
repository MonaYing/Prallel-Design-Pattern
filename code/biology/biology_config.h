#ifndef CONFIG_H_
#define CONFIG_H_

/* Define the message tag id using in different message passing method */
typedef enum _MSGTAG {
  CELL_QUERY_TAG = 900,
  CELL_FINALIZE_TAG,
  CELL_HOP_TAG,
  SQUIRREL_BORN_TAG,
  SQUIRREL_PREPROCESS_TAG,
  SQUIRREL_FINALIZE_TAG,
  SQUIRREL_INFECT_TAG
}MSGTAG;

/* Configuration of the biology model */
#define SIMULATION_ERROR -1 
#define NUM_OF_CELL 16
#define NUM_OF_SQUIRREL 34
#define LIMIT_SQUIRREL 200
#define INIT_SQUIRREL_INFECT 4
#define NUM_OF_HISTORY_STEP 50
#define DEAD_STEP 50
#define LIMIT_OF_MONTH 24
#define STEP_OF_MONTH 2
#define SQUIRREL_SLEEP 10000000

#endif