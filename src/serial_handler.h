// Serial logging handler (mainly for debug mode)
//
// Copyright (c) 2020 Dave Astels

#ifndef __SERIAL_HANDLER_H__
#define __SERIAL_HANDLER_H__

#include <stdbool.h>

bool serial_handler_init(unsigned long timeout);
void serial_handler(const char *level_name, const char *msg);



#endif
