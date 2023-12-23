// Logger
//
// Copyright (c) 2023 Dave Astels

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "logging.h"

#ifndef MAX_NUMBER_OF_HANDLERS
#define MAX_NUMBER_OF_HANDLERS (4)
#endif

int number_of_handlers = 0;
LogHandler log_handlers[MAX_NUMBER_OF_HANDLERS];


const char *level_names[] = {"NOTSET", "DEBUG_DEEP", "DEBUG_MID", "DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL"};
LogLevel log_level;
char log_buffer[1024];

void log_internal_log(LogLevel level, const char *format, va_list args);


LogLevel log_level_for(const char *level_name)
{
     int name_index = -1;
     for (int i = 0; i < 8; i++) {
          if (strcmp(level_name, level_names[i]) == 0) {
               name_index = i;
               break;
          }
     }
     if (name_index == -1) {
          return NOTSET;
     } else {
          return (LogLevel)(name_index);
     }
}


const char *log_name_for_level(LogLevel level)
{
     switch (level) {
     case NOTSET:
          return "NOTSET";
     case DEBUG_DEEP:
          return "DEBUG_DEEP";
     case DEBUG_MID:
          return "DEBUG_MID";
     case DEBUG:
          return "DEBUG";
     case INFO:
          return "INFO";
     case WARNING:
          return "WARNING";
     case ERROR:
          return "ERROR";
     case CRITICAL:
          return "CRITICAL";
     }
     return "BAD_LEVEL";
}


void log_init_logger(LogHandler handler)
{
     log_set_handler(handler);
}


void log_set_handler(LogHandler handler)
{
     number_of_handlers = 1;
     log_handlers[0] = handler;
     log_handlers[1] = NULL;
     log_handlers[2] = NULL;
     log_handlers[3] = NULL;
}


bool log_add_handler(LogHandler handler)
{
     if (number_of_handlers < MAX_NUMBER_OF_HANDLERS) {
          log_handlers[number_of_handlers++] = handler;
          return true;
     } else {
          return false;
     }
}


void log_set_level(LogLevel new_level)
{
     log_level = new_level;
}


LogLevel log_get_level()
{
     return log_level;
}


void _internal_log(LogLevel level, const char *format, va_list args)
{
     if (level >= log_level) {
          vsprintf(log_buffer, format, args);
          for (int i = 0; i < MAX_NUMBER_OF_HANDLERS; i++) {
               if (log_handlers[i]) {
                    log_handlers[i](log_name_for_level(level), log_buffer);
               }
          }
     }
}


void log_raw(LogLevel level, const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(level, format, args);
     va_end (args);
}


void log_debug_deep(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(DEBUG_DEEP, format, args);
     va_end (args);
}


void log_debug_mid(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(DEBUG_MID, format, args);
     va_end (args);
}


void log_debug(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(DEBUG, format, args);
     va_end (args);
}


void log_info(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(INFO, format, args);
     va_end (args);
}


void log_warning(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(WARNING, format, args);
     va_end (args);
}


void log_error(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(ERROR, format, args);
     va_end (args);
}


void log_critical(const char *format, ...)
{
     va_list args;
     va_start (args, format);
     _internal_log(CRITICAL, format, args);
     va_end (args);
}
