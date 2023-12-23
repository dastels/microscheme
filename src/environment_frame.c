/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the environment frame. */

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "environment_frame.h"
#include "logging.h"


environment_frame_t *GLOBAL_ENV;

void init_environments(void);
void add_environment(environment_frame_t *value);
void remove_environment(environment_frame_t *value);


binding_t *binding_with_symbol_and_value(data_t *name, data_t *value)
{
  binding_t *b = (binding_t*)malloc(sizeof(binding_t));
  b->sym = name;
  b->val = value;
  return b;
}

binding_t *find_binding(environment_frame_t *frame, data_t *symbol)
{
  binding_t *binding = dictionary_get(frame->bindings, string_value(symbol));
  if (binding == NULL) {
    if (frame->parent != NULL) {
      return find_binding(frame->parent, symbol);
    } else {
      return NULL;
    }
  } else {
    return binding;
  }
}

binding_t *get_binding(environment_frame_t *frame, data_t *symbol)
{
  return dictionary_get(frame->bindings, string_value(symbol));
}


void set_binding(environment_frame_t *frame, data_t *symbol, binding_t *binding)
{
  dictionary_put(frame->bindings, string_value(symbol), binding);
}



/* ============================================================ */


environment_frame_t *new_environment_frame_below(environment_frame_t *parent_frame)
{
  environment_frame_t *e = (environment_frame_t*)malloc(sizeof(environment_frame_t));
  log_debug("Environment 0x%lX created.", (uintptr_t)e);

  e->parent = parent_frame;
  e->bindings = new_dictionary();
  e->descendants = 0;
  e->in_scope = true;
  if (parent_frame != NULL) {
    parent_frame->descendants++;
  }
  add_environment(e);
  return e;
}


void bind(environment_frame_t *frame, data_t *symbol, data_t *value)
{
  binding_t* binding_or_nil = get_binding(frame, symbol);
  if (binding_or_nil == NULL) {
    binding_t *binding = binding_with_symbol_and_value(symbol, value);
    retain(value);
    set_binding(frame, symbol, binding);
  } else if (frame == GLOBAL_ENV || frame->parent == GLOBAL_ENV) {
    release(binding_or_nil->val);
    retain(value);
    binding_or_nil->val = value;
  }
}


void rebind(environment_frame_t *frame, data_t *symbol, data_t *value)
{
  binding_t* binding_or_nil = get_binding(frame, symbol);
  if (binding_or_nil != NULL) {
    release(binding_or_nil->val);

    retain(value);
    binding_or_nil->val = value;
  }
}


data_t *value_of(environment_frame_t *frame, data_t *symbol)
{
  binding_t *binding_or_nil = find_binding(frame, symbol);
  if (binding_or_nil == NULL) {
    return NULL;
  } else {
    return binding_or_nil->val;
  }
}


void initialize_environment(void)
{
  init_environments();
  GLOBAL_ENV = new_environment_frame_below(NULL);
}


/* void mark_binding(void *data) */
/* { */
/*   mark_cell(((binding_t*)data)->sym); */
/*   mark_cell(((binding_t*)data)->val); */
/* } */


/* void mark_cells_in(environment_frame_t *env) */
/* { */
/*   with_each_value_do(env->bindings, &mark_binding); */
/* } */


void remove_descendant(environment_frame_t *env)
{

  if (env->descendants > 0) {
    env->descendants--;
  }

  log_debug("Removing descendant from environment 0x%lX. Now has %d", (uintptr_t)env, env->descendants);

  if (env->descendants == 0 && !env->in_scope) {
    go_out_of_scope(env);
  }
}


void release_binding(void *data)
{
  release(((binding_t*)data)->val);
}


void clean_environment(environment_frame_t *env)
{
  log_debug("Cleaning environment 0x%lX.", (uintptr_t)env);

  with_each_value_do(env->bindings, &release_binding);
}


void go_out_of_scope(environment_frame_t *env)
{
  if (env == GLOBAL_ENV) {
    return;
  }
  env->in_scope = false;
  if (env->descendants == 0) {
    log_debug("Environment 0x%lX is going out of scope.", (uintptr_t)env);
    remove_descendant(env->parent);
    remove_environment(env);
    clean_environment(env);
    free(env);
  }
}
