/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains utilities. */


#include "data.h"
#include "vector.h"

data_t *vector_to_list_with_tail(Vector*, data_t*);
data_t *vector_to_list(Vector*);
void to_vector(data_t*, Vector*);
data_t *internal_make_list(int count, ...);
data_t *quote_it(data_t *value);
data_t *quote_all(data_t *list);
