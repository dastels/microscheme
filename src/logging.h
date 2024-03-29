// -*- mode: c++ -*-
// Logger
//
// The MIT License (MIT)
//
// Copyright (c) 2020 Dave Astels
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#ifndef __LOGGING_H__
#define __LOGGING_H__

typedef enum { NOTSET, DEBUG_DEEP, DEBUG_MID, DEBUG, INFO, WARNING, ERROR, CRITICAL } LogLevel;

#include <stdarg.h>
#include "logging_handler.h"

typedef void (*LogHandler)(const char *level_name, const char *msg);

const char *log_name_level_for(LogLevel level);
LogLevel log_level_for(const char *level_name);

void log_init_logger(LogHandler handler);
void log_set_level(LogLevel new_level);
void log_set_handler(LogHandler handler);
bool log_add_handler(LogHandler handler);
void log_raw(LogLevel level, const char *format, ...);
void log_debug_deep(const char *format, ...);
void log_debug_mid(const char *format, ...);
void log_debug(const char *format, ...);
void log_info(const char *format, ...);
void log_warning(const char *format, ...);
void log_error(const char *format, ...);
void log_critical(const char *format, ...);

#endif
