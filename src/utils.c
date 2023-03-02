/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains utilities. */

#include <stdlib.h>
#include <stdarg.h>
#include "data.h"
#include "vector.h"

data_t* util_cons(data_t *car, data_t *cdr)
{
  data_t *d = alloc_data(CONS_CELL_TYPE);
  d->data.pair.car_ptr = car;
  d->data.pair.cdr_ptr = cdr;
  return d;
}


void util_append_to_list(data_t *head, data_t *d)
{
  data_t *l = last_cell(head);
  if (l) {
    l->data.pair.cdr_ptr = cons(d, NULL);
    retain(l->data.pair.cdr_ptr);
  }
}


void util_append_to_list_with_no_retain(data_t *head, data_t *d)
{
  data_t *l = last_cell(head);
  if (l) {
    l->data.pair.cdr_ptr = util_cons(d, NULL);
  }
}


data_t *vector_to_list_with_tail(Vector *v, data_t *tail) {
  data_t *head = empty_cons();
  for (int i = 0; i < v->size; i++) {
    util_append_to_list(head, vector_get(v, i));
  }
  retain(tail);
  last_cell(head)->data.pair.cdr_ptr = tail;
  data_t *result = cdr(head);
  free_data(head);
  return result;
}


data_t *vector_to_list(Vector *v) {
  return vector_to_list_with_tail(v, NULL);
}


void to_vector(data_t *l, Vector* v) {
}


data_t *internal_make_list(int count, ...)
{
  va_list args;
  Vector vect;
  vector_init(&vect);

  va_start(args, count);
  for (int i = 0; i < count; ++i) {
    vector_append(&vect, va_arg(args, data_t*));
  }  
  va_end(args);
  data_t *result = vector_to_list(&vect);
  vector_free(&vect);
  return result;
}


data_t *quote_it(data_t *value)
{
  return internal_make_list(2, intern_symbol("quote"), value);
}


data_t *quote_all(data_t *list)
{
  Vector cells;
  vector_init(&cells);
  
  for (data_t *cell = list; cell != NULL; cell = cdr(cell)) {
    vector_append(&cells, quote_it(car(cell)));
  };

  data_t *result = vector_to_list(&cells);
  vector_free(&cells);
  return result;
}

