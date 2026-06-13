#pragma once
#include <string>

#include "Types.h"

class CommandParser {
 public:
  static Command parse(const std::string& input);
};
