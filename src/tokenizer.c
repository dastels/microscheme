 /* Copyright 2015 Dave Astels.  All rights reserved. */
 /* Use of this source code is governed by a BSD-style */
 /* license that can be found in the LICENSE file. */

 /* This package implements a basic LISP interpretor for the ARM Cortex M4 */
 /* This file contains the tokenizer. */

#include "tokenizer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

token_t lookahead_token;
char *lookahead_lit;
char *source;
int position;


void initialize_tokenizer(char *src_string)
{
  source = src_string;
  lookahead_token = ILLEGAL;
  lookahead_lit = (char*)0;
  position = 0;
  consume_token();
}


token_t get_token(void)
{
  return lookahead_token;
}


char *get_lit(void)
{
  return lookahead_lit;
}


int is_eof(void)
{
  return position >= strlen(source);
}


int is_almost_eof()
{
  return position == strlen(source) - 1;
}


int is_letter(char ch)
{
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}


int is_digit(char ch)
{
  return (ch >= '0' && ch <= '9');
}


int is_hex_digit(char ch)
{
  return (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}


int is_symbol_character(char ch)
{
  return is_letter(ch) || is_digit(ch) || ch == '*' || ch == '-' || ch == '?' || ch == '!' || ch == '_' || ch == '>' || ch == ':';
}

int is_space(char ch)
{
  return ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r' || ch == '\f';
}


void extract_lit(int start)
{
  int size = position - start;
  lookahead_lit = malloc(size + 1);
  strncpy(lookahead_lit, source + start, size);
  lookahead_lit[size] = 0;
}


void read_symbol()
{
  int start = position;
  while (!is_eof() && is_symbol_character(source[position])) {
    position++;
  }
  lookahead_token = SYMBOL;
  extract_lit(start);
}


void read_number()
{
  int start = position;
  bool is_hex = false;

  while (!is_eof()) {
    char ch = source[position];
    if (ch == '#') {
      position++;
    } else if ((start == position) && ch == '-') {
      position++;
    } else if (is_digit(ch)) {
      position++;
    } else if ((start == position-1) && ch == 'x') {
      is_hex = true;
      position++;
    } else if (is_hex && is_hex_digit(ch)) {
      position++;
    } else {
      break;
    }
  }

  extract_lit(start);
  if (is_hex) {
    lookahead_token = HEXINTEGER;
  } else {
    lookahead_token = INTEGER;
  }
}


void read_string()
{
  int start = ++position;
  while (!is_eof() && source[position] != '\"') {
    if (source[position] == '\\') {
      position++;
    }
    position++;
  }
  if (is_eof()) {
    lookahead_token = END_OF_FILE;
    lookahead_lit = "";
    return;
  }
  extract_lit(start);
  position++;
  lookahead_token = STRING;
}


void read_next_token(void)
{
  if (is_eof()) {           /* handle END_OF_FILE */
    lookahead_token = END_OF_FILE;
    lookahead_lit = "";
    return;
  }
  while (is_space(source[position])) { /* consume whitespace */
    position++;
    if (is_eof()) {
      lookahead_token = END_OF_FILE;
      lookahead_lit = "";
    }
  }

  char current_char = source[position];
  char next_char = 0;
  if (!is_almost_eof()) {
    next_char = source[position+1];
  }
  if (is_letter(current_char) || current_char == '_') {
    read_symbol();
  } else if (is_digit(current_char)) {
    read_number();
  } else if (current_char == '-' && is_digit(next_char)) {
    read_number();
  } else if (current_char == '#' && next_char == 'x') {
    read_number();
  } else if (current_char == '\"') {
    read_string();
  } else if (current_char == '\'') {
    position++;
    lookahead_token = QUOTE;
    lookahead_lit = "'";
  } else if (current_char == '`') {
    position++;
    lookahead_token = BACKQUOTE;
    lookahead_lit = "`";
  } else if (current_char == ',' && next_char == '@') {
    position += 2;
    lookahead_token = COMMAAT;
    lookahead_lit = ",@";
  } else if (current_char == ',') {
    position++;
    lookahead_token = COMMA;
    lookahead_lit = ",";
  } else if (current_char == '(') {
    position++;
    lookahead_token = LPAREN;
    lookahead_lit = "(";
  } else if (current_char == ')') {
    position++;
    lookahead_token = RPAREN;
    lookahead_lit = ")";
  } else if (current_char == '[') {
    position++;
    lookahead_token = LBRACKET;
    lookahead_lit = "[";
  } else if (current_char == ']') {
    position++;
    lookahead_token = RBRACKET;
    lookahead_lit = "]";
  } else if (current_char == '{') {
    position++;
    lookahead_token = LBRACE;
    lookahead_lit = "{";
  } else if (current_char == '}') {
    position++;
    lookahead_token = RBRACE;
    lookahead_lit = "}";
  } else if (current_char == '.') {
    position++;
    lookahead_token = PERIOD;
    lookahead_lit = ".";
  } else if (current_char == '-' && next_char == '>') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = "->";
  } else if (current_char == '=' && next_char == '>') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = "=>";
  } else if (current_char == '+') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "+";
  } else if (current_char == '-') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "-";
  } else if (current_char == '*') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "*";
  } else if (current_char == '/') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "/";
  } else if (current_char == '%') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "%";
  } else if (current_char == '<' && next_char == '=') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = "<=";
  } else if (current_char == '<') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "<";
  } else if (current_char == '>' && next_char == '=') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = ">=";
  } else if (current_char == '>') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = ">";
  } else if (current_char == '=' && next_char == '=') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = "==";
  } else if (current_char == '=') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "=";
  } else if (current_char == '!' && next_char == '=') {
    position += 2;
    lookahead_token = SYMBOL;
    lookahead_lit = "!=";
  } else if (current_char == '!') {
    position++;
    lookahead_token = SYMBOL;
    lookahead_lit = "!";
  } else if (current_char == '#') {
    position += 2;
    if (next_char == 't') {
      lookahead_token = TRUE;
      lookahead_lit = "#t";
    } else {
      lookahead_token = FALSE;
      lookahead_lit = "#f";
    }
  } else if (current_char == ';') {
    int start = position;
    while (1) {
      if (is_eof()) {
        lookahead_token = COMMENT;
        extract_lit(start);
        return;
      } else if (source[position] == '\n') {
        lookahead_token = COMMENT;
        extract_lit(start);
        return;
      }
      position++;
    }
  } else {
    lookahead_token = ILLEGAL;
    lookahead_lit = malloc(2);
    lookahead_lit[0] = current_char;
    lookahead_lit[1] = '\0';
  }
}


void consume_token(void)
{
  read_next_token();
  if (lookahead_token == COMMENT) {
    consume_token();
  }
}
