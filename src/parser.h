/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the parser. */

#ifndef __PARSER_H

#include "stdlib.h"
#include "tokenizer.h"
#include "data.h"

data_t *parse(char *, char **);
data_t *parse_and_eval(char *source, char **err_ptr);
data_t *parse_and_eval_all(char *source, char **err_ptr);

#define __PARSER_H
#endif
