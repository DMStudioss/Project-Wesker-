#include "CommandParser.h"

#include <algorithm>
#include <sstream>

Command CommandParser::parse(const std::string& input) {
  std::istringstream iss(input);
  std::string verb;
  iss >> verb;
  std::transform(verb.begin(), verb.end(), verb.begin(), ::tolower);

  Command cmd;
  cmd.type = Command::UNKNOWN;

  if (verb == "идти" || verb == "и" || verb == "go")
    cmd.type = Command::MOVE;
  else if (verb == "взять" || verb == "вз" || verb == "take")
    cmd.type = Command::TAKE;
  else if (verb == "использовать" || verb == "исп" || verb == "use")
    cmd.type = Command::USE;
  else if (verb == "атаковать" || verb == "атк" || verb == "attack")
    cmd.type = Command::ATTACK;
  else if (verb == "осмотреться" || verb == "осм" || verb == "look")
    cmd.type = Command::LOOK;
  else if (verb == "инвентарь" || verb == "инв" || verb == "inventory")
    cmd.type = Command::INVENTORY;
  else if (verb == "выход" || verb == "выйти" || verb == "quit")
    cmd.type = Command::QUIT;
  else if (verb == "помощь" || verb == "help")
    cmd.type = Command::HELP;
  else if (verb == "выбросить" || verb == "выкинуть" || verb == "drop")
    cmd.type = Command::DROP;

  std::string arg;
  while (iss >> arg) {
    std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
    cmd.args.push_back(arg);
  }
  return cmd;
}
