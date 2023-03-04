/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the data object. */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include "vector.h"
#include "utils.h"
#include "environment_frame.h"
#include "primitive_function.h"
#include "function.h"
#include "evaluator.h"


data_t *apply_func(function_t *func, data_t *arguments, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  int argument_count = length_of(arguments);
  int expected_number_of_arguments = func->number_of_parameters;
  bool any_number_of_arguments = false;
  bool exact_number_of_arguments = expected_number_of_arguments == argument_count;
  if (!(any_number_of_arguments || exact_number_of_arguments)) {
    char *buf = (char*)malloc((64 + strlen(func->name)) * sizeof(char));
    sprintf(buf, "Wrong number of arguments to %s. Expected %d but got %d.", func->name, expected_number_of_arguments, argument_count);
    *err_ptr = buf;
    return NULL;
  } else {
    environment_frame_t *local_env = new_environment_frame_below(func->env);
    data_t *argument_cell = arguments;
    data_t *parameter_cell = func->parameters;
    while (argument_cell != NULL) {
      data_t *argument_value = evaluate(car(argument_cell), env, err_ptr);
      if (*err_ptr != NULL) {
        go_out_of_scope(local_env);
        return NULL;
      }
      bind(local_env, car(parameter_cell), argument_value);
      parameter_cell = cdr(parameter_cell);
      argument_cell = cdr(argument_cell);
    }

    data_t *result = evaluate_each(func->body, local_env, err_ptr);
    go_out_of_scope(local_env);
    if (*err_ptr != NULL) {
      return NULL;
    }

    return result;
  }
}


data_t *expand(macro_t *macro, data_t *arguments, environment_frame_t *env, char **err_ptr)
{
  int argument_count = length_of(arguments);
  int expected_number_of_arguments = macro->number_of_parameters;
  bool any_number_of_arguments = false;
  bool exact_number_of_arguments = expected_number_of_arguments == argument_count;
  if (!(any_number_of_arguments || exact_number_of_arguments)) {
    char *buf = (char*)malloc((64 + strlen(macro->name)) * sizeof(char));
    sprintf(buf, "Wrong number of arguments to %s. Expected %d but got %d.", macro->name, expected_number_of_arguments, argument_count);
    *err_ptr = buf;
    return NULL;
  } else {
    environment_frame_t *local_env = new_environment_frame_below(macro->env);
    data_t *argument_cell = arguments;
    data_t *parameter_cell = macro->parameters;
    while (argument_cell != NULL) {
      data_t *argument_value = evaluate(car(argument_cell), env, err_ptr);
      if (*err_ptr != NULL) {
        go_out_of_scope(local_env);
        return NULL;
      }
      release(argument_value); /* It's going straight into a binding which will maintain the reference. The one
                                       from evaling is excessive*/

      bind(local_env, car(parameter_cell), argument_value);
      parameter_cell = cdr(parameter_cell);
      argument_cell = cdr(argument_cell);
    }

    data_t *expanded_macro = evaluate(macro->body, local_env, err_ptr);
    go_out_of_scope(local_env);
    if (*err_ptr != NULL) {
      return NULL;
    }
    return expanded_macro;
  }
}


data_t *apply_macro(macro_t *macro, data_t *arguments, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  data_t *expanded_macro = expand(macro, arguments, env, err_ptr);
  data_t *result = evaluate(expanded_macro, env, err_ptr);
  release(expanded_macro);
  if (*err_ptr != NULL) {
    return NULL;
  }

  return result;
}


data_t *apply_prim(primitive_function_t *prim, data_t *arguments, environment_frame_t *env, char **err_ptr)
{
#ifdef DEBUG_TRACE
  printf("Entering %s\n", prim->name);
#endif
  *err_ptr = NULL;
  int argument_count = length_of(arguments);
  int expected_number_of_arguments = prim->number_of_parameters;
  bool any_number_of_arguments = expected_number_of_arguments == -1;
  bool exact_number_of_arguments = expected_number_of_arguments == argument_count;
  if (!(any_number_of_arguments || exact_number_of_arguments)) {
    char *buf = (char*)malloc((64 + strlen(prim->name)) * sizeof(char));
    sprintf(buf, "Wrong number of arguments to %s. Expected %d but got %d.", prim->name, expected_number_of_arguments, argument_count);
    *err_ptr = buf;
    return NULL;
  } else {
    data_t *argument_values;
    if (prim->special_form) {
      argument_values = arguments;
    } else {
      Vector v_arguments;
      vector_init(&v_arguments);
      for (data_t *argument_cell = arguments; argument_cell != NULL; argument_cell = cdr(argument_cell)) {
        data_t *argument_value = evaluate(car(argument_cell), env, err_ptr);
        if (*err_ptr != NULL) {
          return NULL;
        }
        vector_append(&v_arguments, argument_value);
      }
      argument_values = vector_to_list(&v_arguments);
      /* for (int i = 0; i < v_arguments.size; i++) { */
      /*   release(vector_get(&v_arguments, i)); */
      /* } */
      vector_free(&v_arguments);
    }
    data_t *result = prim->impl(argument_values, env, err_ptr);

    if (!prim->special_form) {
      release(argument_values);
    }

    if (*err_ptr != NULL) {
      return NULL;
    }
#ifdef DEBUG_TRACE
    char *str = to_string(result);
    printf("Prim %s returning %s\n", prim->name, str);
    free(str);
#endif
    return result;
  }
}


data_t *evaluate(data_t *sexpr, environment_frame_t *env, char **err_ptr)
{
  data_t *result = NULL;
#ifdef DEBUG_TRACE
  char *str = to_string(sexpr);
  printf("Evaluating %s\n", str);
  free(str);
#endif
  *err_ptr = NULL;
  switch (type_of(sexpr)) {
  case FREE_TYPE:
    result = NULL;
    break;
  case INTEGER_TYPE:
  case UNSIGNED_INTEGER_TYPE:
  case BOOLEAN_TYPE:
  case STRING_TYPE:
  case FUNCTION_TYPE:
  case MACRO_TYPE:
  case PRIMITIVE_TYPE:
    result = sexpr;
    break;
  case SYMBOL_TYPE:
    result = value_of(env, sexpr);
    break;
  case CONS_CELL_TYPE:
    {
      data_t *func_object = value_of(env, car(sexpr));
      if (func_object == NULL) {
        char *name = string_value(car(sexpr));
        char *err_string = malloc(64 + strlen(name) * sizeof(char));
        sprintf(err_string, "Function, special-form, or macro expected for %s. Nothing found.", name);
        *err_ptr = err_string;
        return NULL;
      }
      if (type_of(func_object) == PRIMITIVE_TYPE) {
        primitive_function_t *prim = prim_value(func_object);
        result = apply_prim(prim, cdr(sexpr), env, err_ptr);
        if (*err_ptr != NULL) {
          return NULL;
        } else {
          break;
        }
      } else if (type_of(func_object) == FUNCTION_TYPE) {
        function_t *func = func_value(func_object);
        result = apply_func(func, cdr(sexpr), env, err_ptr);
        if (*err_ptr != NULL) {
          return NULL;
        } else {
          break;
        }
      } else if (type_of(func_object) == MACRO_TYPE) {
        macro_t *macro = macro_value(func_object);
        result = apply_macro(macro, cdr(sexpr), env, err_ptr);
        if (*err_ptr != NULL) {
          return NULL;
        } else {
          break;
        }
      } else {
        *err_ptr = strdup("Function, special-form, or macro expected. Something else found.");
        return NULL;
      }
    }
  }
#ifdef DEBUG_TRACE
  if (freep(result)) {
    printf("HOLY SHIT! EVALUATE RESULTED IN A FREE NODE!!!\n");
  }
  str = to_string(result);
  printf("Evaluate returning %s\n", str);
  free(str);
#endif
  return result;
}


data_t *evaluate_each(data_t *sexprs, environment_frame_t *env, char **err_ptr)
{
  data_t *result = NULL;
  *err_ptr = NULL;
  for (data_t *cell = sexprs; cell != NULL; cell = cdr(cell)) {
    release(result);
    result = evaluate(car(cell), env, err_ptr);
    if (*err_ptr != NULL) {
      return NULL;
    }
  }
  return result;
}
