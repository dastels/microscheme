// -*- c++-mode -*-
//

// ----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include "data.h"
#include "parser.h"
#include "evaluator.h"
#include "special_forms.h"
#include "environment_frame.h"
#include "primitives.h"
#include "logging.h"
#include "serial_handler.h"

void setup_c()
{
     serial_handler_init(0);
     log_init_logger(&serial_handler);
     log_set_level(INFO);
     log_info("Initializing");
     initialize_lisp_data_system();
     initialize_environment();
     register_special_forms();
     register_primitives();
}


void loop_c()
{
}
