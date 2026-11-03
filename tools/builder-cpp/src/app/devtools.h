#pragma once
#include <string>
#include <vector>

// Runs a developer command line mode; -1 when mode is none of them.
int dev_mode(const std::string &mode, const std::vector<std::string> &args);
