/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the data object. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include "dictionary.h"
#include "vector.h"
#include "function.h"
#include "primitive_function.h"
#include "primitives.h"
#include "utils.h"
#include "data.h"

/********************************************************************************/
/* math                                                                         */
/********************************************************************************/

data_t *add_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  int acc = 0;
  for (data_t *cell = args; cell != NULL; cell = cdr(cell)) {
    if (!integerp(car(cell))) {
      *err_ptr = strdup("Add requires integer operands");
      return NULL;
    }
    acc += integer_value(car(cell));
  }
  return integer_with_value(acc);
}


data_t *multiply_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  int acc = 1;
  for (data_t *cell = args; cell != NULL; cell = cdr(cell)) {
    if (!integerp(car(cell))) {
      *err_ptr = strdup("Multiply requires integer operands");
      return NULL;
    }
    acc *= integer_value(car(cell));
  }
  return integer_with_value(acc);
}


data_t *subtract_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  int acc;
  switch (length_of(args)) {
  case 0:
    return integer_with_value(0);
  case 1:
    return integer_with_value(-1 * integer_value(car(args)));
  default:
    acc = integer_value(car(args));
    for (data_t *cell = cdr(args); cell != NULL; cell = cdr(cell)) {
      if (!integerp(car(cell))) {
        *err_ptr = strdup("Subtract requires integer operands");
      return NULL;
      }
      acc -= integer_value(car(cell));
    }
    return integer_with_value(acc);
  }
}


data_t *divide_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  int acc;
  switch (length_of(args)) {
  case 0:
  case 1:
    *err_ptr = strdup("Divide requires at least 2 operands.");
    return NULL;
  default:
    acc = integer_value(car(args));
    for (data_t *cell = cdr(args); cell != NULL; cell = cdr(cell)) {
      if (!integerp(car(cell))) {
        *err_ptr = "Divide requires integer operands";
      return NULL;
      }
      acc /= integer_value(car(cell));
    }
    return integer_with_value(acc);
  }
}


data_t *modulus_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!integerp(car(args)) || !integerp(car(cdr(args)))) {
    *err_ptr = strdup("Modulus requires integer operands");
    return NULL;
  }

  return integer_with_value(integer_value(car(args)) % integer_value(car(cdr(args))));
}


data_t *abs_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!integerp(car(args))) {
    *err_ptr = strdup("abs requires an integer operand");
    return NULL;
  }

  return integer_with_value(abs(integer_value(car(args))));
}


data_t *zero_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  data_t *arg = car(args);
  if (!integerp(arg) && !unsigned_integerp(arg)) {
    *err_ptr = strdup("zero? requires an (unsigned) integer operand");
    return NULL;
  }

  return boolean_with_value(integer_value(arg) == 0);
}


/********************************************************************************/
/* boolean                                                                      */
/********************************************************************************/

data_t *and_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  bool acc = true;
  for (data_t *cell = args; cell != NULL; cell = cdr(cell)) {
    acc = acc && boolean_value(car(cell));
  }
  return boolean_with_value(acc);
}


data_t *or_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  bool acc = false;
  for (data_t *cell = args; cell != NULL; cell = cdr(cell)) {
    acc = acc || boolean_value(car(cell));
  }
  return boolean_with_value(acc);
}


data_t *not_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  return boolean_with_value(!boolean_value(car(args)));
}


/********************************************************************************/
/* binary                                                                       */
/********************************************************************************/


data_t *binary_and_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    *err_ptr = strdup("Binary and requires (unsigned) integer operands");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)) & unsigned_integer_value(car(cdr(args))));
}


data_t *binary_or_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    *err_ptr = strdup("Binary or requires (unsigned) integer operands");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)) | unsigned_integer_value(car(cdr(args))));
}


data_t *binary_not_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args)))) {
    *err_ptr = strdup("Binary not requires an (unsigned) integer operand");
    return NULL;
  }

  return unsigned_integer_with_value(~unsigned_integer_value(car(args)));
}


data_t *binary_xor_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    *err_ptr = strdup("Binary xor requires (unsigned) integer operands");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)) ^ unsigned_integer_value(car(cdr(args))));
}


data_t *left_shift_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    *err_ptr = strdup("Left shift requires (unsigned) integer operands");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)) << integer_value(car(cdr(args))));
}


data_t *right_shift_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    *err_ptr = strdup("Right shift requires (unsigned) integer operands");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)) >> integer_value(car(cdr(args))));
}


/********************************************************************************/
/* conversions                                                                  */
/********************************************************************************/

data_t *integer_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args)))) {
    *err_ptr = strdup("Conversion to integer requires an (unsigned) integer operand");
    return NULL;
  }

  return integer_with_value(integer_value(car(args)));
}


data_t *unsigned_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  if (!(integerp(car(args)) || unsigned_integerp(car(args)))) {
    *err_ptr = strdup("Conversion to unsigned integer requires an (unsigned) integer operand");
    return NULL;
  }

  return unsigned_integer_with_value(unsigned_integer_value(car(args)));
}


/********************************************************************************/
/* list                                                                         */
/********************************************************************************/


data_t *list_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  Vector vect;
  vector_init(&vect);
  for (data_t *cell = args; cell != NULL; cell = cdr(cell)) {
    vector_append(&vect, car(cell));
  }
  return vector_to_list(&vect);
}


data_t *cons_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  return cons(car(args), car(cdr(args)));
}


data_t *car_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  return car(car(args));
}


data_t *cdr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;
  return cdr(car(args));
}


data_t *walk_list(data_t *l, char *path, char **err_ptr) {
  *err_ptr = NULL;
  if (!(listp(l))) {
    *err_ptr = strdup("List access functions require a list operand");
    return NULL;
  }
  for (char *p = path; *p; p++) {
    l = (*p == 'a') ? car(l) : cdr(l);
  }
  return l;
}


data_t *caar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aa", err_ptr);
}


data_t *cadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "da", err_ptr);
}


data_t *cdar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ad", err_ptr);
}


data_t *cddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dd", err_ptr);
}


data_t *caaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aaa", err_ptr);
}


data_t *caadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "daa", err_ptr);
}


data_t *cadar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ada", err_ptr);
}


data_t *caddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dda", err_ptr);
}


data_t *cdaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aad", err_ptr);
}


data_t *cdadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dad", err_ptr);
}


data_t *cddar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "add", err_ptr);
}


data_t *cdddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ddd", err_ptr);
}


data_t *caaaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aaaa", err_ptr);
}


data_t *caaadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "daaa", err_ptr);
}


data_t *caadar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "adaa", err_ptr);
}


data_t *caaddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ddaa", err_ptr);
}


data_t *cadaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aada", err_ptr);
}


data_t *cadadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dada", err_ptr);
}


data_t *caddar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "adda", err_ptr);
}


data_t *cadddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ddda", err_ptr);
}


data_t *cdaaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aaad", err_ptr);
}


data_t *cdaadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "daad", err_ptr);
}


data_t *cdadar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "adad", err_ptr);
}


data_t *cdaddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "ddad", err_ptr);
}


data_t *cddaar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "aadd", err_ptr);
}


data_t *cddadr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dadd", err_ptr);
}


data_t *cdddar_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "addd", err_ptr);
}


data_t *cddddr_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return walk_list(car(args), "dddd", err_ptr);
}


data_t *nth(data_t *l, int n, char **err_ptr)
{
  if (!(listp(l))) {
    *err_ptr = strdup("An initial list is required");
    return NULL;
  }
  if (n < 0) {
    *err_ptr = strdup("A non-negative index is required");
    return NULL;
  }
  if (n >= length_of(l)) {
    *err_ptr = strdup("Index out of bounds");
    return NULL;
  }
  *err_ptr = NULL;
  while (n-- > 0) {
    l = cdr(l);
  }
  return car(l);
}

data_t *listref_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  if (!integerp(car(cdr(args)))) {
    *err_ptr = strdup("list-ref require an integer index");
    return NULL;
  }
  return nth(car(args), integer_value(car(cdr(args))), err_ptr);
}


data_t *first_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 0, err_ptr);
}


data_t *second_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 1, err_ptr);
}


data_t *third_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 2, err_ptr);
}


data_t *fourth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 3, err_ptr);
}


data_t *fifth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 4, err_ptr);
}


data_t *sixth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 5, err_ptr);
}


data_t *seventh_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 6, err_ptr);
}


data_t *eigth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 7, err_ptr);
}


data_t *ninth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 8, err_ptr);
}


data_t *tenth_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return nth(car(args), 9, err_ptr);
}


char *check_list_and_count(data_t *l, data_t *c)
{
  if (!(listp(l))) {
    return "list-head/tail requires an initial list";
  }
  if (!integerp(c)) {
    return "list-head/tail requires an integer count";
  }
  int k = integer_value(c);
  if (k < 0) {
    return "list-head/tail requires a non-negative count";
  }
  if (k > length_of(l)) {
    return "list-head/tail's count is out of bounds";
  }
  return NULL;
}


data_t *listhead_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  data_t * l = car(args);

  *err_ptr = check_list_and_count(l, car(cdr(args)));
  if (*err_ptr != NULL) {
    return NULL;
  }

  int k = integer_value(car(cdr(args)));
  Vector v;
  vector_init(&v);
  while (k-- > 0) {
    vector_append(&v, car(l));
    l = cdr(l);
  }
  data_t *result = vector_to_list(&v);
  vector_free(&v);
  return result;
}


data_t *listtail_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  data_t * l = car(args);
  *err_ptr = check_list_and_count(l, car(cdr(args)));
  if (*err_ptr != NULL) {
    return NULL;
  }

  int k = integer_value(car(cdr(args)));
  while (k-- > 0) {
    l = cdr(l);
  }
  return l;
}


data_t *append_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  switch (length_of(args)) {
  case 0: return NULL;
  case 1: return car(args);
  default:
    {
      Vector v;
      vector_init(&v);
      data_t * arglist = args;
      for (int i = length_of(args) - 1; i > 0; i--) {
        for (data_t *arg = car(arglist); arg != NULL; arg = cdr(arg)) {
          vector_append(&v, car(arg));
        }
        arglist = cdr(arglist);
      }
      data_t *result = vector_to_list_with_tail(&v, car(arglist));
      vector_free(&v);
      return result;
    }
  }
}


data_t *appendbang_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = NULL;

  switch (length_of(args)) {
  case 0:
    return NULL;
  case 1:
    return car(args);
  default:
    {
      data_t *result = car(args);
      data_t *last_list = result;
      data_t * arglist = cdr(args);
      for (int i = length_of(args) - 1; i > 0; i--) {
        if (car(arglist) != NULL) {
          data_t *l;
          for (l = last_list; l != NULL && cdr(l) != NULL; l = cdr(l))
            ;
          last_list = car(arglist);
          set_cdr(l, last_list);
        }
        arglist = cdr(arglist);
      }

      return result;
    }
  }
}


/********************************************************************************/
/* relative                                                                     */
/********************************************************************************/


char *check_relative_args(data_t *args)
{
  if (length_of(args) != 2) {
    return "Relative predicates require exactly 2 arguments.";
  }
  if (!(integerp(car(args)) || unsigned_integerp(car(args))) || !(integerp(car(cdr(args))) || unsigned_integerp(car(cdr(args))))) {
    return "Relative predicates require numeric arguments";
  }
  return NULL;
}


data_t *eq_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  if (length_of(args) != 2) {
    *err_ptr = strdup("Relative functions require exactly 2 arguments.");
    return NULL;
  }

  return boolean_with_value(is_equal(car(args), car(cdr(args))));
}


data_t *neq_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(!is_equal(car(args), car(cdr(args))));
}


data_t *lt_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = check_relative_args(args);
  if (*err_ptr != NULL) {
    return NULL;
  }
  bool result;
  if (integerp(car(args)) || integerp(car(cdr(args)))) {
    result = integer_value(car(args)) < integer_value(car(cdr(args)));
  } else {
    result = unsigned_integer_value(car(args)) < unsigned_integer_value(car(cdr(args)));
  }
  return boolean_with_value(result);
}


data_t *lte_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = check_relative_args(args);
  if (*err_ptr != NULL) {
    return NULL;
  }
  bool result;
  if (integerp(car(args)) || integerp(car(cdr(args)))) {
    result = integer_value(car(args)) <= integer_value(car(cdr(args)));
  } else {
    result = unsigned_integer_value(car(args)) <= unsigned_integer_value(car(cdr(args)));
  }
  return boolean_with_value(result);
}


data_t *gte_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = check_relative_args(args);
  if (*err_ptr != NULL) {
    return NULL;
  }
  bool result;
  if (integerp(car(args)) || integerp(car(cdr(args)))) {
    result = integer_value(car(args)) >= integer_value(car(cdr(args)));
  } else {
    result = unsigned_integer_value(car(args)) >= unsigned_integer_value(car(cdr(args)));
  }
  return boolean_with_value(result);
}


data_t *gt_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  *err_ptr = check_relative_args(args);
  if (*err_ptr != NULL) {
    return NULL;
  }
  bool result;
  if (integerp(car(args)) || integerp(car(cdr(args)))) {
    result = integer_value(car(args)) > integer_value(car(cdr(args)));
  } else {
    result = unsigned_integer_value(car(args)) > unsigned_integer_value(car(cdr(args)));
  }
  return boolean_with_value(result);
}


data_t *nil_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  data_t *thing = car(args);
  return boolean_with_value(thing == NULL || (listp(thing) && length_of(thing) == 0));
}


data_t *listp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(listp(car(args)));
}


data_t *symbolp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(symbolp(car(args)));
}


data_t *stringp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(stringp(car(args)));
}


data_t *integerp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(integerp(car(args)));
}


data_t *unsignedp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(unsigned_integerp(car(args)));
}


data_t *functionp_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(functionp(car(args)));
}


data_t *macrop_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return boolean_with_value(macrop(car(args)));
}


data_t *definition_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  data_t *thing = car(args);

  if (functionp(thing)) {
    return string_with_value(to_string(func_value(thing)->body));
  } else if (macrop(thing)) {
    return string_with_value(to_string(macro_value(thing)->body));
  } else {
    *err_ptr = strdup("definiton requires a function or macro.");
  }
  return NULL;
}


data_t *heap_size_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return integer_with_value(total_cells());
}


data_t *free_size_impl(data_t *args, environment_frame_t *env, char **err_ptr)
{
  return integer_with_value(cells_remaining());
}


/* data_t *gc_impl(data_t *args, environment_frame_t *env, char **err_ptr) */
/* { */
/*   gc(); */
/*   return NULL; */
/* } */


/********************************************************************************/
/* initialization                                                               */
/********************************************************************************/

void register_primitives(void)
{
  register_primitive("+", -1, &add_impl);
  register_primitive("*", -1, &multiply_impl);
  register_primitive("-", -1, &subtract_impl);
  register_primitive("/", -1, &divide_impl);
  register_primitive("%", 2, &modulus_impl);
  register_primitive("abs", 1, &abs_impl);
  register_primitive("zero?", 1, &zero_impl);

  register_primitive("and", -1, &and_impl);
  register_primitive("or", -1, &or_impl);
  register_primitive("not", 1, &not_impl);

  register_primitive("binary-and", 2, &binary_and_impl);
  register_primitive("binary-or", 2, &binary_or_impl);
  register_primitive("binary-not", 1, &binary_not_impl);
  register_primitive("binary-xor", 2, &binary_xor_impl);
  register_primitive("left-shift", 2, &left_shift_impl);
  register_primitive("right-shift", 2, &right_shift_impl);

  register_primitive("integer", 1, &integer_impl);
  register_primitive("unsigned", 1, &unsigned_impl);

  register_primitive("list", -1, &list_impl);
  register_primitive("cons", 2, &cons_impl);
  register_primitive("car", 1, &car_impl);
  register_primitive("cdr", 1, &cdr_impl);

  register_primitive("caar", 1, &caar_impl);
  register_primitive("cadr", 1, &cadr_impl);
  register_primitive("cdar", 1, &cdar_impl);
  register_primitive("cddr", 1, &cddr_impl);

  register_primitive("caaar", 1, &caaar_impl);
  register_primitive("caadr", 1, &caadr_impl);
  register_primitive("cadar", 1, &cadar_impl);
  register_primitive("caddr", 1, &caddr_impl);
  register_primitive("cdaar", 1, &cdaar_impl);
  register_primitive("cdadr", 1, &cdadr_impl);
  register_primitive("cddar", 1, &cddar_impl);
  register_primitive("cdddr", 1, &cdddr_impl);

  register_primitive("caaaar", 1, &caaaar_impl);
  register_primitive("caaadr", 1, &caadar_impl);
  register_primitive("caadar", 1, &cadaar_impl);
  register_primitive("caaddr", 1, &caddar_impl);
  register_primitive("cadaar", 1, &cdaaar_impl);
  register_primitive("cadadr", 1, &cdadar_impl);
  register_primitive("caddar", 1, &cddaar_impl);
  register_primitive("cadddr", 1, &cdddar_impl);
  register_primitive("cdaaar", 1, &caaadr_impl);
  register_primitive("cdaadr", 1, &caaddr_impl);
  register_primitive("cdadar", 1, &cadadr_impl);
  register_primitive("cdaddr", 1, &cadddr_impl);
  register_primitive("cddaar", 1, &cdaadr_impl);
  register_primitive("cddadr", 1, &cdaddr_impl);
  register_primitive("cdddar", 1, &cddadr_impl);
  register_primitive("cddddr", 1, &cddddr_impl);

  register_primitive("list-ref", 2, &listref_impl);
  register_primitive("first", 1, &first_impl);
  register_primitive("second", 1, &second_impl);
  register_primitive("third", 1, &third_impl);
  register_primitive("fourth", 1, &fourth_impl);
  register_primitive("fifth", 1, &fifth_impl);
  register_primitive("sixth", 1, &sixth_impl);
  register_primitive("seventh", 1, &seventh_impl);
  register_primitive("eigth", 1, &eigth_impl);
  register_primitive("ninth", 1, &ninth_impl);
  register_primitive("tenth", 1, &tenth_impl);

  register_primitive("list-head", 2, &listhead_impl);
  register_primitive("list-tail", 2, &listtail_impl);

  register_primitive("append", -1, &append_impl);
  register_primitive("append!", -1, &appendbang_impl);

  register_primitive("eq?", 2, &eq_impl);
  register_primitive("neq?", 2, &neq_impl);
  register_primitive("<", 2, &lt_impl);
  register_primitive("<=", 2, &lte_impl);
  register_primitive(">", 2, &gt_impl);
  register_primitive(">=", 2, &gte_impl);

  register_primitive("nil?", 1, &nil_impl);
  register_primitive("list?", 1, &listp_impl);
  register_primitive("symbol?", 1, &symbolp_impl);
  register_primitive("string?", 1, &stringp_impl);
  register_primitive("integer?", 1, &integerp_impl);
  register_primitive("unsigned?", 1, &unsignedp_impl);
  register_primitive("function?", 1, &functionp_impl);
  register_primitive("macro?", 1, &macrop_impl);

  register_primitive("definition", 1, &definition_impl);
  register_primitive("heap-size", 0, &heap_size_impl);
  register_primitive("free-size", 0, &free_size_impl);

  /* register_primitive("gc", 0, &gc_impl); */
}
