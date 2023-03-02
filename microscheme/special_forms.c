/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains special form implementations. */

#include <stdlib.h>
#include <strings.h>
#include "vector.h"
#include "environment_frame.h"
#include "data.h"
#include "utils.h"
#include "evaluator.h"
#include "primitive_function.h"
#include "function.h"


data_t *lambda_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *arg_names = car(args);
  data_t *body = cdr(args);

  if (!all_of_type(SYMBOL_TYPE, arg_names)) {
    *err_ptr = strdup("All argument names must be symbols");
    return NULL;
  }

  return func_with_value(make_function("anonymous", arg_names, body, env));
}


data_t *define_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *declaration = car(args);
  if (symbolp(declaration)) {
    data_t *value = evaluate(car(cdr(args)), env, err_ptr);
    bind(env, declaration, value);
    return value;
  } else if (listp(declaration)) {
    data_t *name = car(declaration);
    if (!symbolp(name)) {
      *err_ptr = strdup("Function names must be symbols");
      return NULL;
    }
    data_t *arg_names = cdr(declaration);
    
    if (!all_of_type(SYMBOL_TYPE, arg_names)) {
      *err_ptr = strdup("All argument names must be symbols");
      return NULL;
    }
    data_t *body = cdr(args);
    
    data_t *func = func_with_value(make_function(string_value(name), arg_names, body, env));
    bind(env, name, func);
    return func;
  } else {
    *err_ptr = strdup("Invalid definition");
    return NULL;
  }
}


data_t *defmacro_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *declaration = car(args);
  if (listp(declaration)) {
    data_t *name = car(declaration);
    data_t *params = cdr(declaration);
    if (!symbolp(name)) {
      *err_ptr = strdup("Macro name has to be a symbol");
      return NULL;
    }
    data_t *body = car(cdr(args));
    data_t *macro = macro_with_value(make_macro(string_value(name), params, body, env));
    bind(env, name, macro);
    return macro;
  } else {
    *err_ptr = strdup("Invalid macro definition");
    return NULL;
  }
}


data_t *cond_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  for (data_t *clauses = args; clauses != NULL; clauses = cdr(clauses)) {
    data_t *clause = car(clauses);
    data_t *predicate = car(clause);
    if (symbolp(predicate) && strcmp(string_value(predicate), "else") == 0) {
      data_t *result = evaluate_each(cdr(clause), env, err_ptr);
      return (*err_ptr != NULL) ? NULL : result;
    } else {
      data_t *predicate_value = evaluate(predicate, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      if (boolean_value(predicate_value)) {
        data_t *result = evaluate_each(cdr(clause), env, err_ptr);
        return (*err_ptr != NULL) ? NULL : result;
      }
    }
  }
  return NULL;
}


data_t *if_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  data_t *true_clause = car(cdr(args));
  data_t *false_clause = car(cdr(cdr(args)));

  if (length_of(args) > 3) {
    *err_ptr = strdup("if accepts up to 3 arguments");
    return NULL;
  }

  data_t *condition = evaluate(car(args), env, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }

  if (boolean_value(condition)) {
    if (true_clause != NULL) {
      return evaluate(true_clause, env, err_ptr);
    }
    return NULL;
  } else {
    if (false_clause != NULL) {
      return evaluate(false_clause, env, err_ptr);
    }
    return NULL;
  }
}


data_t *let_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  environment_frame_t *local_env = new_environment_frame_below(env);
  data_t *bindings = car(args);
  for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
    data_t *binding = car(binding_cell);
    data_t *binding_name = car(binding);
    if (!symbolp(binding_name)) {
      *err_ptr = strdup("let requires symbols as binding names");
      go_out_of_scope(local_env);
      return NULL;
    }
    data_t *binding_value = evaluate(car(cdr(binding)), env, err_ptr);
    if (*err_ptr != NULL) {
      go_out_of_scope(local_env);
      return NULL;
    }
    bind(local_env, binding_name, binding_value);
  }
  
  data_t *result = evaluate_each(cdr(args), local_env, err_ptr);
  if (*err_ptr != NULL) {
    go_out_of_scope(local_env);
    return NULL;
  }
  
  go_out_of_scope(local_env);
  return result;
}


data_t *letstar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  environment_frame_t *local_env = new_environment_frame_below(env);
  data_t *bindings = car(args);
  for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
    data_t *binding = car(binding_cell);
    data_t *binding_name = car(binding);
    if (!symbolp(binding_name)) {
      *err_ptr = strdup("let* requires symbols as binding names");
      go_out_of_scope(local_env);
      return NULL;
    }
    data_t *binding_value = evaluate(car(cdr(binding)), local_env, err_ptr);
    if (*err_ptr != NULL) {
      go_out_of_scope(local_env);
      return NULL;
    }
    bind(local_env, binding_name, binding_value);
  }
  
  data_t *result = evaluate_each(cdr(args), local_env, err_ptr);
  if (*err_ptr != NULL) {
    go_out_of_scope(local_env);
    return NULL;
  }
  
  go_out_of_scope(local_env);
  return result;
}


data_t *letrec_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  environment_frame_t *local_env = new_environment_frame_below(env);
  data_t *bindings = car(args);
  for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
    data_t *binding_name = car(car(binding_cell));
    if (!symbolp(binding_name)) {
      *err_ptr = strdup("letrec requires symbols as binding names");
      return NULL;
    }
    bind(local_env, binding_name, NULL);
  }

  for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
    data_t *binding = car(binding_cell);
    data_t *binding_name = car(binding);
    data_t *binding_value = evaluate(car(cdr(binding)), local_env, err_ptr);
    if (*err_ptr != NULL) {
      return NULL;
    }
    rebind(local_env, binding_name, binding_value);
  }
  
  data_t *result = evaluate_each(cdr(args), local_env, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }
  
  return result;
}


data_t *set_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  if (!symbolp(car(args))) {
    *err_ptr = strdup("set! requires a symbol as it's first argument.");
    return NULL;
  }
  data_t *value = evaluate(car(cdr(args)), env, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }
  rebind(env, car(args), value);
  return value;
}


data_t *quote_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  return car(args);
}


data_t *process_quasiquoted(data_t *sexpr, int level, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  if (!listp(sexpr)) {
    return cons(sexpr, NULL);
  }

  if (car(sexpr) == intern_symbol("quasiquote")) {
    data_t *processed = process_quasiquoted(car(cdr(sexpr)), level + 1, env, err_ptr);
    if (*err_ptr != NULL) {
      return NULL;
    }
    return cons(cons(intern_symbol("quasiquote"), processed), NULL);
  } else if (car(sexpr) == intern_symbol("unquote")) {
    if (level == 1) {
      data_t *processed = process_quasiquoted(car(cdr(sexpr)), level, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      data_t *result = evaluate(car(processed), env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return cons(result, NULL);
      return NULL;
    } else {
      data_t *processed = process_quasiquoted(car(cdr(sexpr)), level - 1, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return cons(cons(intern_symbol("unquote"), processed), NULL);
    }
  } else if (car(sexpr) == intern_symbol("unquote-splicing")) {
    if (level == 1) {
      data_t *processed = process_quasiquoted(car(cdr(sexpr)), level, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      data_t *result = evaluate(car(processed), env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return result;
    } else {
      data_t *processed = process_quasiquoted(car(cdr(sexpr)), level - 1, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return cons(cons(intern_symbol("unquote-splicing"), processed), NULL);
    }
  } else {
    Vector parts;
    vector_init(&parts);
    for (data_t *cell = sexpr; cell != NULL; cell = cdr(cell)) {
      data_t *processed = process_quasiquoted(car(cell), level, env, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      vector_append(&parts, processed);
    }
    data_t *flat = flatten(vector_to_list(&parts));
    vector_free(&parts);
    return cons(flat, NULL);
  }
}


data_t *quasiquote_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *result = process_quasiquoted(car(args), 1, env, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }
  return car(result);
}


data_t *unquote_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = strdup("unquote should not be used outside of a quasiquoted expression.");
  return NULL;
}


data_t *unquote_splicing_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = strdup("unquote-splicing should not be used outside of a quasiquoted expression.");
  return NULL;
}


data_t *expand_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *m = evaluate(car(args), env, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }

  if (!macrop(m)) {
    *err_ptr = strdup("expand expects a macro as it's parameter.");
    return NULL;
  }
 return expand(macro_value(m), cdr(args), env, err_ptr);
}


data_t *do_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  if (length_of(args) < 2) {
    *err_ptr = strdup("do requires at least bindings and a termination clause.");
    return NULL;
  }
    
  data_t *bindings = car(args);
  if (!listp(bindings)) {
    *err_ptr = strdup("do bindings must be a list.");
    return NULL;
  }

  environment_frame_t *local_env = new_environment_frame_below(env);
  for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
    data_t *binding = car(binding_cell);
    data_t *binding_name = car(binding);
    if (!symbolp(binding_name)) {
      *err_ptr = strdup("do requires symbols as binding names");
      go_out_of_scope(local_env);
      return NULL;
    }
    data_t *binding_value = evaluate(car(cdr(binding)), env, err_ptr);
    if (*err_ptr != NULL) {
      go_out_of_scope(local_env);
      return NULL;
    }
    bind(local_env, binding_name, binding_value);
  }
  
  data_t *termination = car(cdr(args));
  if (!listp(termination)) {
    *err_ptr = strdup("do termination clause must be a list");
    go_out_of_scope(local_env);
    return NULL;
  }    
  
  data_t *body = car(cdr(cdr(args)));
  while (true) {
    data_t *condition = evaluate(car(termination), local_env, err_ptr);
    if (*err_ptr != NULL) {
      go_out_of_scope(local_env);
      return NULL;
    }
    if (boolean_value(condition)) {
      data_t *result = evaluate_each(cdr(termination), local_env, err_ptr);
      go_out_of_scope(local_env);
      if (*err_ptr != NULL) {
        return NULL;
      } else {
        return result;
      }
    }

    if (body != NULL) {
      evaluate_each(body, local_env, err_ptr);
    }

    for (data_t *binding_cell = bindings; binding_cell != NULL; binding_cell = cdr(binding_cell)) {
      data_t *binding = car(binding_cell);
      data_t *binding_name = car(binding);
      data_t *new_value = evaluate(car(cdr(cdr(binding))), local_env, err_ptr);
      if (*err_ptr != NULL) {
        go_out_of_scope(local_env);
        return NULL;
      }
      rebind(local_env, binding_name, new_value);
    }
  }
}


void register_special_forms(void)
{
  register_special_form("lambda", -1, &lambda_impl);
  register_special_form("define", -1, &define_impl);
  register_special_form("defmacro", 2, &defmacro_impl);
  register_special_form("cond", -1, &cond_impl);
  register_special_form("if", -1, &if_impl);
  register_special_form("let", -1, &let_impl);
  register_special_form("let*", -1, &letstar_impl);
  register_special_form("letrec", -1, &letrec_impl);
  register_special_form("set!", 2, &set_impl);
  register_special_form("quote", 1, &quote_impl);
  register_special_form("quasiquote", 1, &quasiquote_impl);
  register_special_form("unquote", 1, &unquote_impl);
  register_special_form("unquote-splicing", 1, &unquote_splicing_impl);
  register_special_form("expand", -1, &expand_impl);
  register_special_form("do", -1, &do_impl);
}
