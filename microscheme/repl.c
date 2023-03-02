#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include "data.h"
#include "parser.h"
#include "evaluator.h"
#include "special_forms.h"
#include "environment_frame.h"
#include "primitives.h"

 	
static char *line_read = (char *)NULL;

char *rl_gets (char *prompt)
{
  if (line_read) {
    free (line_read);
    line_read = (char *)NULL;
  }

  line_read = readline (prompt);

  if (line_read && *line_read) {
    add_history (line_read);
  }

  return (line_read);
}


int
main(int argc, char* argv[])
{
  char *err;
  using_history();
  read_history("./.history");
  initialize_lisp_data_system();
  initialize_environment();
  register_special_forms();
  register_primitives();


  printf("Welcome to ZombieWizard Embedded Lisp: a Lisp for embedded ARM MCUs.\n");
  printf("Copyright 2015 ZombieWizard. All rights reserved.\n\n");
  printf("heap size: %d, allocated: %d, remaining: %d\n\n", total_cells(), cells_allocated(), cells_remaining());

  while (1) {
    char *line = rl_gets("> ");
    if (line && *line) {
      if (strcmp(line, "(quit)") == 0) {
        break;
      }
      data_t *sexpr = parse(line, &err);
      if (err != NULL) {
        printf("ERROR: %s\n", err);
        free(err);
      } else {        
        data_t *result = evaluate(sexpr, GLOBAL_ENV, &err);
        if (err) {
          printf("ERROR: %s\n", err);
          free(err);
        } else {
          char *result_string = to_string(result);
          printf("==> %s\n", result_string);
          free(result_string);
          dump_node(result, heap_index(result));
          release(sexpr);
          if (unreferencedp(result)) {
            release(result);
          }
          printf("heap size: %d, allocated: %d, remaining: %d\n\n", total_cells(), cells_allocated(), cells_remaining());
          //          dump_active_heap();
        }
      }
    }
  }

  write_history("./.history");
}
