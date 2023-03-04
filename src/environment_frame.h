/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the environment frame. */

#ifndef __ENVIRONMENT_FRAME_H
#define __ENVIRONMENT_FRAME_H

#include "dictionary.h"
#include "data.h"

typedef struct binding_t {
  data_t *sym;
  data_t *val;
} binding_t;

typedef struct environment_frame_t {
  int descendants;
  bool in_scope;
  struct environment_frame_t *parent;
  dictionary_t *bindings;
} environment_frame_t;

extern environment_frame_t *GLOBAL_ENV;


void initialize_environment(void);
environment_frame_t *new_environment_frame_below(environment_frame_t*);
void bind(environment_frame_t *frame, data_t *symbol, data_t *value);
void rebind(environment_frame_t *frame, data_t *symbol, data_t *value);
data_t *value_of(environment_frame_t *frame, data_t *symbol);
/* void mark_cells_in(environment_frame_t *env); */
void go_out_of_scope(environment_frame_t *env);

#endif
