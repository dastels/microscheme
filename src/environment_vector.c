// vector.c

#include <stdio.h>
#include <stdlib.h>
#include "environment_vector.h"


EnvVector environments;

void init_environments(void)
{
  env_vector_init(&environments);
}


EnvVector *get_environments(void)
{
  return &environments;
}


void add_environment(environment_frame_t *e)
{
  env_vector_add(&environments, e);
}


void remove_environment(environment_frame_t *e)
{
  env_vector_remove(&environments, e);
}


void env_vector_init(EnvVector *vector) {
  // initialize size and capacity
  vector->size = 0;
  vector->capacity = ENV_VECTOR_INITIAL_CAPACITY;

  // allocate memory for vector->data
  vector->data = malloc(sizeof(EnvVector*) * vector->capacity);
}


void env_vector_add(EnvVector *vector, environment_frame_t *value) {
  for (int i = 0; i < vector->size; i++) {
    if (vector->data[i] == NULL) {
      vector->data[i] = value;
      return;
    }
  }

  // make sure there's room to expand into
  env_vector_double_capacity_if_full(vector);

  // append the value and increment vector->size
  vector->data[vector->size++] = value;
}


void env_vector_remove(EnvVector *vector, environment_frame_t *value)
{
  for (int i = 0; i < vector->size; i++) {
    if (vector->data[i] == value) {
      vector->data[i] = NULL;
    }
  }
}


environment_frame_t *env_vector_get(EnvVector *vector, int index) {
  if (index >= vector->size || index < 0) {
    printf("Index %d out of bounds for environment vector of size %d\n", index, vector->size);
    exit(1);
  }
  return vector->data[index];
}


void env_vector_double_capacity_if_full(EnvVector *vector) {
  if (vector->size >= vector->capacity) {
    // double vector->capacity and resize the allocated memory accordingly
    vector->capacity *= 2;
    vector->data = realloc(vector->data, sizeof(data_t*) * vector->capacity);
  }
}


void env_vector_free(EnvVector *vector) {
  free(vector->data);
}
