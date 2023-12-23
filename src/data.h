/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the data object. */


#ifndef __DATA_H
#define __DATA_H

#include <stdbool.h>
#include "primitive_function.h"
#include "function.h"
#include "macro.h"
#include "environment_frame.h"


#define FREE_TYPE 0
#define CONS_CELL_TYPE 1
#define INTEGER_TYPE 2
#define UNSIGNED_INTEGER_TYPE 3
#define BOOLEAN_TYPE 4
#define STRING_TYPE 5
#define SYMBOL_TYPE 6
#define FUNCTION_TYPE 7
#define MACRO_TYPE 8
#define PRIMITIVE_TYPE 9


typedef struct data_t {
  struct {
    __uint8_t type : 4;
    __uint8_t refs : 4;
  } meta;
  union {
    __int32_t int_data;
    __uint32_t uint_data;
    char *string_data;
    bool boolean_data;
    struct {
      struct data_t *car_ptr;
      struct data_t *cdr_ptr;
    } pair;
    primitive_function_t *prim_func;
    function_t *func;
    macro_t *macro;
    struct data_t *next;
  } data;
} data_t;

extern data_t *LISP_FALSE;
extern data_t *LISP_TRUE;

void initialize_lisp_data_system(void);

data_t *alloc_data(__uint8_t);
void free_data(data_t*);
void release(data_t*);
data_t *retain(data_t*);
bool unreferencedp(data_t*);
int total_cells(void);
int cells_allocated(void);
int cells_remaining(void);
void dump_node(data_t*, int);
void dump_active_heap(void);
int heap_index(data_t*);
/* void gc(void); */

data_t *intern_symbol(char*);

__uint8_t type_of(data_t*);
char *type_name(int type_number);
bool all_of_type(int, data_t*);

data_t *integer_with_value(int);
int integer_value(data_t*);

data_t *unsigned_integer_with_value(__uint32_t);
__uint32_t unsigned_integer_value(data_t*);

data_t *string_with_value(char*);
char *string_value(data_t*);

data_t *boolean_with_value(bool);
bool boolean_value(data_t*);

primitive_function_t *make_primitive_function(char*, int, bool, primitive_function_impl);
data_t *prim_with_value(primitive_function_t*);
primitive_function_t *prim_value(data_t*);

function_t *make_function(char*, data_t*, data_t*, environment_frame_t*);
data_t *func_with_value(function_t*);
function_t *func_value(data_t*);

macro_t *make_macro(char*, data_t*, data_t*, environment_frame_t*);
data_t *macro_with_value(macro_t*);
macro_t *macro_value(data_t*);

data_t *car(data_t*);
data_t *cdr(data_t*);
void set_car(data_t*, data_t*);
void set_cdr(data_t*, data_t*);
data_t *flatten(data_t *l);
void append_to_list(data_t*, data_t*);
data_t *last_cell(data_t*);
data_t *empty_cons(void);
data_t* cons(data_t*, data_t*);
int length_of(data_t*);
char *to_string(data_t*);
bool is_equal(data_t*, data_t*);

bool freep(data_t*);
bool symbolp(data_t*);
bool stringp(data_t*);
bool integerp(data_t*);
bool unsigned_integerp(data_t*);
bool listp(data_t*);
bool functionp(data_t*);
bool macrop(data_t*);

/* void mark_cell(data_t*); */

#endif
