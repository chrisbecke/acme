#include "console.hpp"

#include <stdarg.h>
#include <stdio.h>

using namespace anode;

void console::logf(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);

  vprintf(fmt, args);

  va_end(fmt);
  printf("\r\n");
}
