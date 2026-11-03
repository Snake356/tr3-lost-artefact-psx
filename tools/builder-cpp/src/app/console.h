// Output to the console the builder was started from (it is a window program: command line modes
// write to the parent's console or to the pipe they were given).
#pragma once
#include <string>

void console_attach();
void console_write(const std::string &s);
