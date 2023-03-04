/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the primitive function support. */

#ifndef __PRIMITIVE_FUNCTION_H
#define __PRIMITIVE_FUNCTION_H

typedef struct data_t data_t;
typedef struct environment_frame_t environment_frame_t;

typedef data_t*(*primitive_function_impl)(data_t *args, environment_frame_t *env, char **err_ptr);

typedef struct primitive_function_t {
  char *name;
  int number_of_parameters;
  bool special_form;
  primitive_function_impl impl;
} primitive_function_t;


void register_primitive(char *name, int parameter_count, primitive_function_impl impl);
void register_special_form(char *name, int parameter_count, primitive_function_impl impl);

#endif
