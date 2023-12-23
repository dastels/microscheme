// -*- c++-mode -*-
//

// ----------------------------------------------------------------------------


// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
// #pragma GCC diagnostic push
// #pragma GCC diagnostic ignored "-Wunused-parameter"
// #pragma GCC diagnostic ignored "-Wmissing-declarations"
// #pragma GCC diagnostic ignored "-Wreturn-type"

#include <Arduino.h>

extern "C" void setup_c();
extern "C" void loop_c();

void setup() { setup_c(); }
void loop()  { loop_c(); }

// #pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
