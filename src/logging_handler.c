// Abstract logging handler
//
// Copyright (c) 2020 Dave Astels

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "logging_handler.h"


char _buffer[256];


/*
 Returns the current time.
*/

char *time_stamp(){
     char *timestamp = (char *)malloc(sizeof(char) * 16);
     time_t ltime;
     ltime = time(NULL);
     struct tm *tm;
     tm = localtime(&ltime);

     sprintf(timestamp,"%04d/%02d/%02d %02d:%02d:%02d", tm->tm_year+1900, tm->tm_mon,
             tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
     return timestamp;
}


char *log_format(const char *level_name, const char *msg)
{
     char *tod = time_stamp();
     sprintf(_buffer, "%s - %10s: %s", tod, level_name, msg);
     free(tod);
     return _buffer;
}
