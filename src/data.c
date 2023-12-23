/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the data object. */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "dictionary.h"
#include "vector.h"
#include "environment_vector.h"
#include "function.h"
#include "primitive_function.h"
#include "utils.h"
#include "data.h"
#include "logging.h"

#define INITIAL_HEAP_SIZE (64 * 1024)
#define SMALL_INTEGER_CACHE_SIZE 32

data_t *small_integer_cache[SMALL_INTEGER_CACHE_SIZE];

data_t *heap = NULL;
data_t *free_list = NULL;
int total_cell_count = 0;
int free_cell_count = 0;
dictionary_t* interned_symbols;

data_t *LISP_FALSE;
data_t *LISP_TRUE;


int total_cells(void)
{
     return total_cell_count;
}


int cells_remaining(void)
{
     return free_cell_count;
}


int cells_allocated(void)
{
     return total_cell_count - free_cell_count;
}


/* Returns a cell to the free list */

void free_data(data_t *d)
{
     char * str = to_string(d);
     log_debug_deep("Freeing a %s: %s.", type_name(type_of(d)), str);
     free(str);
     d->meta.type = FREE_TYPE;
     d->data.next = free_list;
     free_list = d;
     free_cell_count++;
}


void release_function(function_t *f)
{
     free(f->name);
     release(f->parameters);
     release(f->body);
     f->env->descendants--;
}


void release_macro(macro_t *m)
{
     free(m->name);
     release(m->parameters);
     release(m->body);
     m->env->descendants--;
}


bool is_cached_int(data_t *d)
{
     return integerp(d) && integer_value(d) >= 0 && integer_value(d) < SMALL_INTEGER_CACHE_SIZE;
}


bool reference_counting_exempt(data_t *d)
{
     return d == NULL || type_of(d) == FREE_TYPE || type_of(d) == SYMBOL_TYPE || type_of(d) == PRIMITIVE_TYPE || type_of(d) == BOOLEAN_TYPE || is_cached_int(d);
}

data_t *retain(data_t *d) {
     if (!reference_counting_exempt(d)) {
          d->meta.refs++;
          char *str = to_string(d);
          log_debug_deep("Retaining a %s: %s. Reference count now %d", type_name(type_of(d)), str, d->meta.refs);
          free(str);
     }
     return d;
}


/* Releases used by the cell (other than itself) and adds it to the
   free list */

void release(data_t *d)
{
     if (freep(d)) {
          char* str = to_string(d);
          log_critical("TRYING TO RELEASE FREED DATA!  %s", str);
          free(str);
          return;
     }

     if (reference_counting_exempt(d)) {
          return;
     }

     char* str = to_string(d);
     log_debug_deep("Releasing a %s: %s", type_name(type_of(d)), str);
     free(str);

     if (d->meta.refs == 0 || --d->meta.refs == 0) {
               char* str = to_string(d);
               log_debug_deep("Reference count = 0. Freeing.", type_name(type_of(d)), str);
               free(str);

          switch (type_of(d)) {
          case STRING_TYPE:
               free(d->data.string_data);
               break;
          case CONS_CELL_TYPE:
               release(car(d));
               release(cdr(d));
               d->data.pair.car_ptr = NULL;
               d->data.pair.cdr_ptr = NULL;
               break;
          case FUNCTION_TYPE:
               release_function(func_value(d));
               break;
          case MACRO_TYPE:
               release_macro(macro_value(d));
               break;
          default:
               break;
          }

          free_data(d);
     } else {
          char* str = to_string(d);
          log_debug_deep("Decremented ref count of a %s: %s Now %d.", type_name(type_of(d)), str, d->meta.refs);
          free(str);
     }
}


bool unreferencedp(data_t *d)
{
     return !freep(d) && d->meta.refs == 0;
}


void dump_node(data_t* d, int node_index)
{
     printf("Node %d\n", node_index);
     if (freep(d)) {
          printf("  free\n");
     } else {
          printf("  %d references\n", d->meta.refs);
          char *str = to_string(d);
          printf("  %s - %s\n", type_name(type_of(d)), str);
          free(str);
     }
}

void dump_active_heap(void)
{
     data_t *node = heap;
     for (int i = 0; i < total_cell_count; i++) {
          if (!freep(node)) {
               dump_node(node, i);
          }
          node++;
     }
}


int heap_index(data_t* d)
{
     return (d - heap) / sizeof(data_t);
}

/* void gc(void) */
/* { */
/*   printf("Collecting garbage\n"); */

/*   /\* clear marks *\/ */
/*   for (int i = 0; i < total_cell_count; heap[i++].meta.marked = 0) */
/*     ; */

/*   EnvVector *environments = get_environments(); */

/*   /\* mark cells referenced from all environments *\/ */
/*   printf("Marking environments"); */
/*   for (int i = 0; i < environments->size; i++) { */
/*     if (env_vector_get(environments, i) != NULL) { */
/*       printf("."); */
/*       mark_cells_in(env_vector_get(environments, i)); */
/*     } */
/*   } */
/*   printf("\n"); */

/*   /\* sweep, reclaiming unmarked cells *\/ */
/*   for (int i = 0; i < total_cell_count; i++) { */
/*     if (!heap[i].meta.marked) { */
/*       free_data(&heap[i]); */
/*     } */
/*   } */
/* } */


/* void mark_cell(data_t *cell) */
/* { */
/*   if (!cell->meta.marked) { */
/*     cell->meta.marked = 1; */
/*     if (listp(cell)) { */
/*       mark_cell(cell->data.pair.car_ptr); */
/*       mark_cell(cell->data.pair.cdr_ptr); */
/*     } */
/*   } */
/* } */


/* Fetch a cell from the free list */

data_t *alloc_data(__uint8_t the_type)
{
     /* if (free_list == NULL) { */
     /*   gc(); */
     /* } */

     log_debug_deep("Allocating a %s. ", type_name(the_type));

     if (free_list == NULL) {
          log_critical("Could not allocate data object");
          exit(-1);
     }
     data_t *d = free_list;
     d->meta.type = the_type;
     d->meta.refs = 0;
     free_list = free_list->data.next;
     free_cell_count--;
     return d;
}

bool allocate_heap(int heapsize)
{
     /* allocate a heap */
     heap = (data_t*)malloc(heapsize);
     if (heap == NULL) {
          return false;
     }
     total_cell_count = free_cell_count = heapsize / sizeof(data_t);

     /* create the free list */
     data_t *tail = heap;
     data_t *rest = tail + 1;
     for (int i = 1; i < total_cell_count; i++) {
          tail->meta.type = FREE_TYPE;
          tail->data.next = rest;
          tail++;
          rest++;
     }
     tail->data.next = NULL;
     free_list = heap;

     return true;
}


/* Initialization  */

data_t *internal_boolean_with_value(bool value)
{
     data_t *d = alloc_data(BOOLEAN_TYPE);
     d->data.boolean_data = value;
     return d;
}


void initialize_lisp_data_system(void)
{
     allocate_heap(INITIAL_HEAP_SIZE);
     log_info("Allocated heap of %d cells, each %lu bytes.", total_cells(), sizeof(data_t));
     LISP_FALSE = internal_boolean_with_value(false);
     LISP_TRUE = internal_boolean_with_value(true);
     interned_symbols = new_dictionary();
     log_info("Pre allocating %d small integers.", SMALL_INTEGER_CACHE_SIZE);
     for (int i = 0; i < SMALL_INTEGER_CACHE_SIZE; i++) {
          small_integer_cache[i] = alloc_data(INTEGER_TYPE);
          small_integer_cache[i]->data.int_data = (__int32_t)i;
     }

}


data_t *symbol_with_name(char *name)
{
     data_t *d = alloc_data(SYMBOL_TYPE);
     d->data.string_data = name;
     return d;
}

data_t *intern_symbol(char *name)
{
     data_t *sym = dictionary_get(interned_symbols, name);
     if (sym == NULL) {
          sym = symbol_with_name(name);
          dictionary_put(interned_symbols, name, sym);
     }
     return sym;
}


data_t *boolean_with_value(bool value)
{
     return value ? LISP_TRUE : LISP_FALSE;
}


bool boolean_value(data_t *d)
{
     if (d->meta.type != BOOLEAN_TYPE) {
          return false;
     } else {
          return d->data.boolean_data;
     }
}


char *type_name(int type_number)
{
     switch(type_number) {
     case FREE_TYPE:             return "free";
     case CONS_CELL_TYPE:        return "cons";
     case INTEGER_TYPE:          return "int";
     case UNSIGNED_INTEGER_TYPE: return "uint";
     case BOOLEAN_TYPE:          return "bool";
     case STRING_TYPE:           return "str";
     case SYMBOL_TYPE:           return "sym";
     case FUNCTION_TYPE:         return "func";
     case MACRO_TYPE:            return "mac";
     case PRIMITIVE_TYPE:        return "prim";
     default:                    return "??";
     }
}

__uint8_t type_of(data_t *d)
{
     if (d == NULL) {
          return -1;
     }
     return d->meta.type;
}


bool all_of_type(int required_type, data_t *list)
{
     for (data_t *cell = list; cell != NULL; cell = cdr(cell)) {
          if (type_of(car(cell)) != required_type) {
               return false;
          }
     }
     return true;
}


data_t *integer_with_value(int value)
{
     if (value >= 0 && value < SMALL_INTEGER_CACHE_SIZE) {
          return small_integer_cache[value];
     } else {
          data_t *d = alloc_data(INTEGER_TYPE);
          d->data.int_data = (__int32_t)value;
          return d;
     }
}


int integer_value(data_t *d)
{
     if (integerp(d) || unsigned_integerp(d)) {
          return (int)(d->data.int_data);
     } else {
          return 0;
     }
}


data_t *unsigned_integer_with_value(__uint32_t value)
{
     data_t *d = alloc_data(UNSIGNED_INTEGER_TYPE);
     d->data.uint_data = value;
     return d;
}


__uint32_t unsigned_integer_value(data_t *d)
{
     if (integerp(d) || unsigned_integerp(d)) {
          return (d->data.uint_data);
     } else {
          return 0;
     }
}


primitive_function_t *make_primitive_function(char *name, int parameter_count, bool special, primitive_function_impl impl)
{
     data_t *sym = intern_symbol(name);
     primitive_function_t *prim = (primitive_function_t *)malloc(sizeof(primitive_function_t));
     prim->name = name;
     prim->number_of_parameters = parameter_count;
     prim->special_form = special;
     prim->impl = impl;
     return prim;
}


data_t *prim_with_value(primitive_function_t *func)
{
     data_t *d = alloc_data(PRIMITIVE_TYPE);
     d->data.prim_func = func;
     return d;
}


primitive_function_t *prim_value(data_t* d)
{
     if (type_of(d) != PRIMITIVE_TYPE) {
          return NULL;
     } else {
          return d->data.prim_func;
     }
}


function_t *make_function(char *name, data_t *parameters, data_t *body, environment_frame_t *env)
{
     function_t *func = (function_t *)malloc(sizeof(function_t));
     retain(parameters);
     retain(body);
     func->name = name;
     func->number_of_parameters = length_of(parameters);
     func->parameters = parameters;
     func->body = body;
     env->descendants++;
     func->env = env;
     return func;
}


data_t *func_with_value(function_t *func)
{
     data_t *d = alloc_data(FUNCTION_TYPE);
     d->data.func = func;
     return d;
}


function_t *func_value(data_t* d)
{
     if (type_of(d) != FUNCTION_TYPE) {
          return NULL;
     } else {
          return d->data.func;
     }
}


macro_t *make_macro(char *name, data_t *parameters, data_t *body, environment_frame_t *env)
{
     macro_t *macro = (macro_t *)malloc(sizeof(macro_t));
     retain(parameters);
     retain(body);
     macro->name = name;
     macro->number_of_parameters = length_of(parameters);
     macro->parameters = parameters;
     macro->body = body;
     env->descendants++;
     macro->env = env;
     return macro;
}


data_t *macro_with_value(macro_t *macro)
{
     data_t *d = alloc_data(MACRO_TYPE);
     d->data.macro = macro;
     return d;
}


macro_t *macro_value(data_t* d)
{
     if (type_of(d) != MACRO_TYPE) {
          return NULL;
     } else {
          return d->data.macro;
     }
}


data_t *string_with_value(char *value)
{
     data_t *d = alloc_data(STRING_TYPE);
     d->data.string_data = value;
     return d;
}


data_t *empty_cons(void)
{
     data_t *d = alloc_data(CONS_CELL_TYPE);
     d->data.pair.car_ptr = NULL;
     d->data.pair.cdr_ptr = NULL;
     return d;
}


data_t* cons(data_t *car, data_t *cdr)
{
     data_t *d = alloc_data(CONS_CELL_TYPE);
     retain(car);
     retain(cdr);
     d->data.pair.car_ptr = car;
     d->data.pair.cdr_ptr = cdr;
     return d;
}


data_t *last_cell(data_t *l)
{
     while (l && l->data.pair.cdr_ptr) {
          l = l->data.pair.cdr_ptr;
     }
     return l;
}


void append_to_list(data_t *head, data_t *d)
{
     data_t *l = last_cell(head);
     if (l) {
          l->data.pair.cdr_ptr = cons(d, NULL);
     }
}


int length_of(data_t *d)
{
     if (type_of(d) != CONS_CELL_TYPE) {
          return 0;
     }

     int len = 0;
     while (d != NULL) {
          d = cdr(d);
          len++;
     }
     return len;
}


char *string_value(data_t *d)
{
     if (d->meta.type != STRING_TYPE && d->meta.type != SYMBOL_TYPE) {
          return "";
     } else {
          return d->data.string_data;
     }
}


data_t *car(data_t *d)
{
     if (d == NULL || d->meta.type != CONS_CELL_TYPE) {
          return NULL;
     } else {
          return d->data.pair.car_ptr;
     }
}


data_t *cdr(data_t *d)
{
     if (d == NULL || d->meta.type != CONS_CELL_TYPE) {
          return NULL;
     } else {
          return d->data.pair.cdr_ptr;
     }
}


void set_car(data_t *d, data_t *e)
{
     d->data.pair.car_ptr = e;
}


void set_cdr(data_t *d, data_t *e)
{
     d->data.pair.cdr_ptr = e;
}


data_t *flatten(data_t *l)
{
     Vector v;
     vector_init(&v);
     for (data_t *outer_cell = l; outer_cell != NULL; outer_cell = cdr(outer_cell)) {
          if (listp(car(outer_cell))) {
               for (data_t *inner_cell = car(outer_cell); inner_cell != NULL; inner_cell = cdr(inner_cell)) {
                    vector_append(&v, car(inner_cell));
               }
          } else {
               vector_append(&v, car(outer_cell));
          }
     }
     data_t *result = vector_to_list(&v);
     vector_free(&v);
     return result;
}


void debug_point(void)
{
}

char *list_to_string(data_t *d)
{
     if (car(d) == d || cdr(d) == d) {
          log_error("Self referential cons");
          debug_point();
     }
     char **strings = (char**)malloc(length_of(d) * sizeof(char**));
     int len = 3;
     int index = 0;
     for (data_t *cell = d; cell != NULL; cell = cdr(cell)) {
          char *s = to_string(car(cell));
          strings[index++] = s;
          len += strlen(s) + 1;
     }
     char *buf = (char*)malloc(len * sizeof(char));
     buf[0] = '(';
     int offset = 1;
     for (int i = 0; i < length_of(d); i++) {
          strcpy(buf + offset, strings[i]);
          offset += strlen(strings[i]) + 1;
          buf[offset-1] = ' ';
          free(strings[i]);
     }
     buf[len-2] = ')';
     buf[len-1] = 0;
     free(strings);
     return buf;
}


char *int_to_string(data_t *d)
{
     int digits = 2;
     int temp = integer_value(d);
     while (temp > 0) {
          digits++;
          temp /= 10;
     }
     char *buf = (char*)malloc(digits * sizeof(char));
     sprintf(buf, "%d", integer_value(d));
     return buf;
}


char *uint_to_string(data_t *d)
{
     char *buf = (char*)malloc(11 * sizeof(char));
     sprintf(buf, "#x%08x", unsigned_integer_value(d));
     return buf;
}


char *bool_to_string(data_t *d)
{
     char *buf = (char*)malloc(3 * sizeof(char));
     sprintf(buf, "#%s", boolean_value(d) ? "t" : "f");
     return buf;
}


char *string_to_string(data_t *d)
{
     char *buf = (char*)malloc((strlen(string_value(d)) + 3) * sizeof(char));
     sprintf(buf, "\"%s\"", string_value(d));
     return buf;
}


char *func_to_string(data_t *d)
{
     function_t *f = func_value(d);
     char *buf = (char*)malloc((9 + strlen(f->name)) * sizeof(char));
     sprintf(buf, "<func: %s>", f->name);
     return buf;
}


char *macro_to_string(data_t *d)
{
     macro_t *m = macro_value(d);
     char *buf = (char*)malloc((10 + strlen(m->name)) * sizeof(char));
     sprintf(buf, "<macro: %s>", m->name);
     return buf;
}


char *prim_to_string(data_t *d)
{
     primitive_function_t *p = prim_value(d);
     char *buf = (char*)malloc((9 + strlen(p->name)) * sizeof(char));
     sprintf(buf, "<prim: %s>", p->name);
     return buf;
}


char *to_string(data_t *d)
{
     if (d == NULL) {
          return strdup("nil");
     }

     switch (type_of(d)) {
     case FREE_TYPE:             return strdup("Free-object");
     case CONS_CELL_TYPE:        return list_to_string(d);
     case INTEGER_TYPE:          return int_to_string(d);
     case UNSIGNED_INTEGER_TYPE: return uint_to_string(d);
     case BOOLEAN_TYPE:          return bool_to_string(d);
     case STRING_TYPE:           return string_to_string(d);
     case SYMBOL_TYPE:           return strdup(string_value(d));
     case FUNCTION_TYPE:         return func_to_string(d);
     case MACRO_TYPE:            return macro_to_string(d);
     case PRIMITIVE_TYPE:        return prim_to_string(d);
     default:                    return strdup("unknown data type");
     }
}


bool is_equal(data_t *d, data_t *o)
{
     if (d == o) {
          return true;
     }

     if (d == NULL || o == NULL) {
          return false;
     }

     if (type_of(d) != type_of(o)) {
          return false;
     }

     if (listp(d)) {
          if (length_of(d) != length_of(o)) {
               return false;
          }
          data_t *a1 = d;
          data_t *a2 = o;
          while (a1 != NULL) {
               if (!is_equal(car(a1), car(a2))) {
                    return false;
               }
               a1 = cdr(a1);
               a2 = cdr(a2);
          }
          return true;
     }

     switch (type_of(d)) {
     case INTEGER_TYPE:          return integer_value(d) == integer_value(o);
     case UNSIGNED_INTEGER_TYPE: return unsigned_integer_value(d) == unsigned_integer_value(o);
     case BOOLEAN_TYPE:          return boolean_value(d) == boolean_value(o);
     case STRING_TYPE:           return strcmp(string_value(d), string_value(o)) == 0;
     case FUNCTION_TYPE:         return func_value(d) == func_value(o);
     case MACRO_TYPE:            return macro_value(d) == macro_value(o);
     case PRIMITIVE_TYPE:        return prim_value(d) == prim_value(o);
     default:                    return false;
     }
}


bool check_type(data_t *d, __uint8_t t)
{
     return type_of(d) == t;
}


bool freep(data_t *d)
{
     return check_type(d, FREE_TYPE);
}


bool symbolp(data_t *d)
{
     return check_type(d, SYMBOL_TYPE);
}


bool stringp(data_t *d)
{
     return check_type(d, STRING_TYPE);
}


bool integerp(data_t *d)
{
     return check_type(d, INTEGER_TYPE);
}


bool unsigned_integerp(data_t *d)
{
     return check_type(d, UNSIGNED_INTEGER_TYPE);
}


bool listp(data_t *d)
{
     return d == NULL || check_type(d, CONS_CELL_TYPE);
}


bool functionp(data_t *d)
{
     return check_type(d, FUNCTION_TYPE) || check_type(d, PRIMITIVE_TYPE);
}


bool macrop(data_t *d)
{
     return check_type(d, MACRO_TYPE);
}
