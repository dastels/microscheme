/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the macro support. */

#ifndef __MACRO_H
#define __MACRO_H

typedef struct data_t data_t;
typedef struct environment_frame_t environment_frame_t;

typedef struct macro_t {
  char *name;
  int number_of_parameters;
  data_t *parameters;
  environment_frame_t *env;
  data_t *body;
} macro_t;

#endif
