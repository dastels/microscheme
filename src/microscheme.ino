// -*- c++-mode -*-
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
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

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-parameter"
// #pragma GCC diagnostic ignored "-Wmissing-declarations"
// #pragma GCC diagnostic ignored "-Wreturn-type"

void setup()
{
  initialize_lisp_data_system();
  initialize_environment();
  register_special_forms();
  register_primitives();
}


void loop()
{
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------


// /home/dastels/.arduino15/packages/adafruit/tools/arm-none-eabi-gcc/9-2019q4/bin/arm-none-eabi-g++ -L/tmp/arduino_build_63692 -Os -Wl,--gc-sections -save-temps -T/home/dastels/.arduino15/packages/adafruit/hardware/samd/1.7.11/variants/grand_central_m4/linker_scripts/gcc/flash_with_bootloader.ld -Wl,-Map,/tmp/arduino_build_63692/microscheme.ino.map --specs=nano.specs --specs=nosys.specs -mcpu=cortex-m4 -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -o /tmp/arduino_build_63692/microscheme.ino.elf /tmp/arduino_build_63692/sketch/data.c.o /tmp/arduino_build_63692/sketch/dictionary.c.o /tmp/arduino_build_63692/sketch/environment_frame.c.o /tmp/arduino_build_63692/sketch/environment_vector.c.o /tmp/arduino_build_63692/sketch/evaluator.c.o /tmp/arduino_build_63692/sketch/hash.c.o /tmp/arduino_build_63692/sketch/parser.c.o /tmp/arduino_build_63692/sketch/primitive_function.c.o /tmp/arduino_build_63692/sketch/primitives.c.o /tmp/arduino_build_63692/sketch/repl.c.o /tmp/arduino_build_63692/sketch/special_forms.c.o /tmp/arduino_build_63692/sketch/tokenizer.c.o /tmp/arduino_build_63692/sketch/utils.c.o /tmp/arduino_build_63692/sketch/vector.c.o /tmp/arduino_build_63692/sketch/microscheme.ino.cpp.o /tmp/arduino_build_63692/core/variant.cpp.o -Wl,--start-group -L/home/dastels/.arduino15/packages/adafruit/tools/CMSIS/5.4.0/CMSIS/Lib/GCC/ -L/home/dastels/.arduino15/packages/adafruit/hardware/samd/1.7.11/variants/grand_central_m4 -larm_cortexM4lf_math -mfloat-abi=hard -mfpu=fpv4-sp-d16 -L/home/dastels/.arduino15/packages/adafruit/hardware/samd/1.7.11/variants/grand_central_m4 -lm /tmp/arduino_build_63692/../arduino_cache_619128/core/core_963d13d037c279622e24152430657ffe.a -Wl,--end-group
