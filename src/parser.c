/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the parser. */

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "strings.h"
#include "vector.h"
#include "utils.h"
#include "data.h"
#include "evaluator.h"
#include "parser.h"
#include "tokenizer.h"
#include "utils.h"
#include "logging.h"

data_t *make_integer(char *lit, char **err_ptr)
{
  int i;
  if (sscanf(lit, "%d", &i) == EOF) {
    *err_ptr = strdup("Unexpected EOF while reading an integer");
    return NULL;
  } else {
    *err_ptr = NULL;
    return integer_with_value(i);
  }
}


data_t *make_hex_integer(char *lit, char **err_ptr)
{
  __uint32_t i;
  *lit = '0';
  if (sscanf(lit, "%x", &i) == EOF) {
    *err_ptr = strdup("Unexpected EOF while reading a hex integer");
    return NULL;
  } else {
    *err_ptr = NULL;
    return unsigned_integer_with_value(i);
  }
}


data_t *make_string(char *lit, char **err_ptr)
{
  *err_ptr = NULL;
  return string_with_value(lit);
}


data_t *make_symbol(char *lit, char **err_ptr)
{
  *err_ptr = NULL;
  return intern_symbol(lit);
}

data_t *parse_expression(bool *, char **); /* forward ref */

data_t *parse_cons_cell(bool *eof, char **err_ptr)
{
  *err_ptr = NULL;
  token_t token = get_token();
  if (token == RPAREN) {
    consume_token();
    return NULL;
  }
  data_t *car_ptr;
  data_t *cdr_ptr;
  Vector cells;
  vector_init(&cells);

  while (token != RPAREN) {
    if (token == PERIOD) {
      consume_token();
      cdr_ptr = parse_expression(eof, err_ptr);
      if (*eof || *err_ptr != NULL) {
        return NULL;
      }
      token = get_token();
      if (token != RPAREN) {
        *err_ptr = strdup("Expected ')'");
      }
      consume_token();
      return vector_to_list_with_tail(&cells, cdr_ptr);
    } else {
      car_ptr = parse_expression(eof, err_ptr);
      if (*eof) {
        *err_ptr = strdup("Unexpected EOF (expected a closing parenthesis)");
        return NULL;
      }
      if (*err_ptr != NULL) {
        return NULL;
      }
      vector_append(&cells, car_ptr);
    }
    token = get_token();
  }
  consume_token();
  data_t *result = vector_to_list(&cells);
  vector_free(&cells);
  return result;
}


data_t *parse_expression(bool *eof, char **err_ptr)
{
  data_t *d = NULL;
  *err_ptr = NULL;
  *eof = false;
  while (true) {
    token_t tok = get_token();
    char *lit = get_lit();
    switch (tok) {
    case END_OF_FILE:
      *eof = true;
      return NULL;
    case COMMENT:
      consume_token();
      break;
    case INTEGER:
      consume_token();
      d = make_integer(lit, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return d;
    case HEXINTEGER:
      consume_token();
      d = make_hex_integer(lit, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return d;
    case STRING:
      consume_token();
      d = make_string(lit, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return d;
    case TRUE:
      consume_token();
      return LISP_TRUE;
    case FALSE:
      consume_token();
      return LISP_FALSE;
    case SYMBOL:
      consume_token();
      d = make_symbol(lit, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return d;
    case LPAREN:
      consume_token();
      d = parse_cons_cell(eof, err_ptr);
      if (*err_ptr != NULL) {
        return NULL;
      }
      return d;
    case QUOTE:
      consume_token();
      return internal_make_list(2, intern_symbol("quote"), parse_expression(eof, err_ptr));
    case BACKQUOTE:
      consume_token();
      return internal_make_list(2, intern_symbol("quasiquote"), parse_expression(eof, err_ptr));
    case COMMA:
      consume_token();
      return internal_make_list(2, intern_symbol("unquote"), parse_expression(eof, err_ptr));
    case COMMAAT:
      consume_token();
      return internal_make_list(2, intern_symbol("unquote-splicing"), parse_expression(eof, err_ptr));
    case PERIOD:
    case ILLEGAL:
      return NULL;
    default:
      return NULL;
    }
  }
}


data_t *parse(char *source, char **err_ptr)
{
  log_debug("Starting parse");
  *err_ptr = NULL;
  bool eof_flag = false;
  initialize_tokenizer(source);
  data_t *result = parse_expression(&eof_flag, err_ptr);
  log_debug("Ending parse");
  return result;
}


data_t *parse_and_eval(char *source, char **err_ptr)
{
  data_t *sexpr = parse(source, err_ptr);
  if (*err_ptr != NULL) {
    return NULL;
  }
  data_t *result = evaluate(sexpr, GLOBAL_ENV, err_ptr);
  release(sexpr);
  if (*err_ptr != NULL) {
    return NULL;
  }
  return result;
}


data_t *parse_and_eval_all(char *source, char **err_ptr)
{
  *err_ptr = NULL;
  bool eof_flag= false;
  data_t *sexpr;
  data_t *result = NULL;
  initialize_tokenizer(source);
  while (!eof_flag) {
    release(result);
    sexpr = parse_expression(&eof_flag, err_ptr);
    if (*err_ptr != NULL) {
      return NULL;
    }
    if (!eof_flag) {
      result = evaluate(sexpr, GLOBAL_ENV, err_ptr);
      release(sexpr);
      if (*err_ptr != NULL) {
        return NULL;
      }
    }
  }
  return result;
}
