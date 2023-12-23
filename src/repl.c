#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string.h>
#include <unistd.h>
#include "data.h"
#include "parser.h"
#include "evaluator.h"
#include "special_forms.h"
#include "environment_frame.h"
#include "primitives.h"
#include "logging.h"
#include "serial_handler.h"


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
     char c;
     char *log_level = "ERROR";
     char *expr = NULL;
     while ((c = getopt (argc, argv, "l:e:")) != -1) {
          switch (c)
          {
          case 'l':
               log_level = optarg;
               break;
          case 'e':
               expr = optarg;
               break;
          }
     }

     char *err;
     serial_handler_init(0);
     log_init_logger(&serial_handler);
     log_set_level(ERROR);
     log_info("Initializing");

     using_history();
     read_history("./.history");
     initialize_lisp_data_system();
     initialize_environment();
     register_special_forms();
     register_primitives();

     log_set_level(log_level_for(log_level));

     if (expr) {
          log_debug("heap size: %d, allocated: %d, remaining: %d", total_cells(), cells_allocated(), cells_remaining());
           data_t *sexpr = parse(expr, &err);
           if (err != NULL) {
                log_error("%s", err);
                free(err);
           } else {
                data_t *result = evaluate(sexpr, GLOBAL_ENV, &err);
                if (err) {
                     log_error("%s", err);
                     free(err);
                } else {
                     char *result_string = to_string(result);
                     printf("%s\n", result_string);
                     free(result_string);
                     release(sexpr);
                     if (unreferencedp(result)) {
                          release(result);
                     }
                     log_debug("heap size: %d, allocated: %d, remaining: %d", total_cells(), cells_allocated(), cells_remaining());
                }
           }
     } else {
          printf("\n\nWelcome to ZombieWizard Embedded Lisp.\n");
          printf("Copyright 2015-2023 Dave Astels. All rights reserved.\n\n");
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
     }
     write_history("./.history");
}
