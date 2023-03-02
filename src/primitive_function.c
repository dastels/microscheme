/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the primitive function support. */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "data.h"
#include "utils.h"
#include "environment_frame.h"
#include "primitive_function.h"


void register_primitive(char *name, int arg_count, primitive_function_impl impl)
{
  bind(GLOBAL_ENV, intern_symbol(name), prim_with_value(make_primitive_function(name, arg_count, false, impl)));
}


void register_special_form(char *name, int arg_count, primitive_function_impl impl)
{
  bind(GLOBAL_ENV, intern_symbol(name), prim_with_value(make_primitive_function(name, arg_count, true, impl)));
}


