// vector.h

#ifndef __ENV_VECTOR_H
#define __ENV_VECTOR_H

#include "environment_frame.h"
#include "data.h"

#define ENV_VECTOR_INITIAL_CAPACITY 100

// Define a vector type
typedef struct {
  int size;      // slots used so far
  int capacity;  // total available slots
  environment_frame_t **data;     // array of environments we're storing
} EnvVector;

EnvVector *get_environments(void);

void env_vector_init(EnvVector *vector);
void env_vector_add(EnvVector *vector, struct environment_frame_t *value);
struct environment_frame_t *env_vector_get(EnvVector *vector, int index);
void env_vector_double_capacity_if_full(EnvVector *vector);
void env_vector_remove(EnvVector *vector, struct environment_frame_t *value);
void env_vector_free(EnvVector *vector);

#endif
