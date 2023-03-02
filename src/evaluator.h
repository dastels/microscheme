/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the evaluator. */

#ifndef __EVALUATOR_H
#define __EVALUATOR_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "data.h"
#include "environment_frame.h"
#include "function.h"
#include "primitive_function.h"

data_t *apply_func(function_t *func, data_t *args, environment_frame_t *env, char **err_ptr);
data_t *apply_prim(primitive_function_t *prim, data_t *args, environment_frame_t *env, char **err_ptr);

data_t *expand(macro_t *macro, data_t *arguments, environment_frame_t *env, char **err_ptr);
data_t *apply_macro(macro_t *macro, data_t *arguments, environment_frame_t *env, char **err_ptr);

data_t *evaluate(data_t *sexpr, environment_frame_t *env, char **err_ptr);
data_t *evaluate_each(data_t *sexpr, environment_frame_t *env, char **err_ptr);
#endif
