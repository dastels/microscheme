// Serial logging handler (mainly for debug mode)
//
// Copyright (c) 2020 Dave Astels

#include <stdbool.h>
#include <stdio.h>
#include "logging_handler.h"
#include "serial_handler.h"


bool serial_handler_init(unsigned long timeout)
{
     /* Serial.begin(115200); */
     /* unsigned long timeout_time = millis() + timeout; */
     /* while (millis() < timeout_time) { */
     /*      if (Serial) { */
     /*           return true; */
     /*      } */
     /* } */
     /* return false; */
     return true;
}


void serial_handler(const char *level_name, const char *msg)
{
     /* Serial.println(log_format(level_name, msg)); */
     /* Serial.flush(); */
     printf("%s\n", log_format(level_name, msg));
}
