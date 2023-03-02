/* Copyright 2015 Dave Astels.  All rights reserved. */
/* Use of this source code is governed by a BSD-style */
/* license that can be found in the LICENSE file. */

/* This package implements a basic LISP interpretor for the ARM Cortex M4 */
/* This file contains the tokenizer. */


#ifndef __TOKENIZER_H

typedef enum {
  ILLEGAL,
  SYMBOL,
  INTEGER,
  HEXINTEGER,
  STRING,
  QUOTE,
  BACKQUOTE,
  COMMA,
  COMMAAT,
  LPAREN,
  RPAREN,
  LBRACKET,
  RBRACKET,
  LBRACE,
  RBRACE,
  PERIOD,
  TRUE,
  FALSE,
  COMMENT,
  END_OF_FILE
} token_t;

void initialize_tokenizer(char *src_string);
token_t get_token(void);
char *get_lit(void);
void consume_token(void);

#define __TOKENIZER_H
#endif
